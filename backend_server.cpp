#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
using Socket = SOCKET;
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
using Socket = int;
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#endif

namespace
{
    constexpr int kDefaultPort = 8080;
    constexpr std::size_t kMaxRequestBytes = 1024 * 1024;
    constexpr double kFixedCharge = 50.0;
    constexpr double kTaxRate = 0.18;
    constexpr const char *kCustomerFile = "customers_api.dat";
    constexpr const char *kTariffFile = "tariffs_api.dat";

    struct Tariff
    {
        double domestic = 5.0;
        double commercial = 7.5;
        double industrial = 10.0;
    };

    struct Customer
    {
        int id = 0;
        std::string name;
        std::string address;
        std::string contact;
        std::string type = "Domestic";
        double previousReading = 0.0;
        double currentReading = 0.0;
        double unitsConsumed = 0.0;
        double billAmount = 0.0;
        bool isPaid = false;
    };

    struct Bill
    {
        double units = 0.0;
        double energy = 0.0;
        double fixed = kFixedCharge;
        double tax = 0.0;
        double total = 0.0;
    };

    struct HttpRequest
    {
        std::string method;
        std::string path;
        std::map<std::string, std::string> headers;
        std::string body;
    };

    std::vector<Customer> customers;
    Tariff tariff;

    std::string trim(const std::string &value)
    {
        const auto begin = std::find_if_not(value.begin(), value.end(), [](unsigned char ch)
                                            { return std::isspace(ch) != 0; });
        const auto end = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char ch)
                                          { return std::isspace(ch) != 0; })
                             .base();
        return begin >= end ? "" : std::string(begin, end);
    }

    std::string lower(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch)
                       { return static_cast<char>(std::tolower(ch)); });
        return value;
    }

    int configuredPort()
    {
        const char *rawPort = std::getenv("PORT");
        if (!rawPort || std::string(rawPort).empty())
            return kDefaultPort;

        try
        {
            const int parsed = std::stoi(rawPort);
            if (parsed > 0 && parsed <= 65535)
                return parsed;
        }
        catch (...)
        {
        }

        return kDefaultPort;
    }

    std::string jsonEscape(const std::string &value)
    {
        std::ostringstream out;
        for (char ch : value)
        {
            switch (ch)
            {
            case '\\':
                out << "\\\\";
                break;
            case '"':
                out << "\\\"";
                break;
            case '\n':
                out << "\\n";
                break;
            case '\r':
                out << "\\r";
                break;
            case '\t':
                out << "\\t";
                break;
            default:
                out << ch;
                break;
            }
        }
        return out.str();
    }

    std::string unescapeJsonString(const std::string &value)
    {
        std::string result;
        bool escaping = false;
        for (char ch : value)
        {
            if (escaping)
            {
                if (ch == 'n')
                    result.push_back('\n');
                else if (ch == 'r')
                    result.push_back('\r');
                else if (ch == 't')
                    result.push_back('\t');
                else
                    result.push_back(ch);
                escaping = false;
            }
            else if (ch == '\\')
            {
                escaping = true;
            }
            else
            {
                result.push_back(ch);
            }
        }
        return result;
    }

    bool extractString(const std::string &body, const std::string &key, std::string &value)
    {
        const std::regex pattern("\"" + key + "\"\\s*:\\s*\"((?:\\\\.|[^\"])*)\"");
        std::smatch match;
        if (!std::regex_search(body, match, pattern))
            return false;
        value = unescapeJsonString(match[1].str());
        return true;
    }

    bool extractDouble(const std::string &body, const std::string &key, double &value)
    {
        const std::regex pattern("\"" + key + "\"\\s*:\\s*(-?\\d+(?:\\.\\d+)?)");
        std::smatch match;
        if (!std::regex_search(body, match, pattern))
            return false;
        value = std::stod(match[1].str());
        return true;
    }

    bool extractInt(const std::string &body, const std::string &key, int &value)
    {
        double parsed = 0.0;
        if (!extractDouble(body, key, parsed))
            return false;
        value = static_cast<int>(parsed);
        return true;
    }

    bool extractBool(const std::string &body, const std::string &key, bool &value)
    {
        const std::regex pattern("\"" + key + "\"\\s*:\\s*(true|false)");
        std::smatch match;
        if (!std::regex_search(body, match, pattern))
            return false;
        value = match[1].str() == "true";
        return true;
    }

    double rateForType(const std::string &type)
    {
        if (type == "Commercial")
            return tariff.commercial;
        if (type == "Industrial")
            return tariff.industrial;
        return tariff.domestic;
    }

    Bill calculateBill(double previous, double current, const std::string &type)
    {
        Bill bill;
        bill.units = std::max(0.0, current - previous);
        bill.energy = bill.units * rateForType(type);
        bill.tax = (bill.energy + bill.fixed) * kTaxRate;
        bill.total = bill.energy + bill.fixed + bill.tax;
        return bill;
    }

    void recalculate(Customer &customer)
    {
        const Bill bill = calculateBill(customer.previousReading, customer.currentReading, customer.type);
        customer.unitsConsumed = bill.units;
        customer.billAmount = bill.total;
    }

    std::vector<std::string> split(const std::string &line, char delimiter)
    {
        std::vector<std::string> parts;
        std::string item;
        std::istringstream input(line);
        while (std::getline(input, item, delimiter))
            parts.push_back(item);
        return parts;
    }

    void seedCustomers()
    {
        customers = {
            {1001, "Aarav Sharma", "14 Lakeview Residency, Pune", "+91 98765 12001", "Domestic", 1850, 2015, 165, 1032.5, true},
            {1002, "Blue Orbit Mart", "22 Market Spine Road, Nashik", "+91 98765 12002", "Commercial", 7120, 7435, 315, 2843.5, false},
            {1003, "Vega Steel Works", "Sector 8 Industrial Belt, Nagpur", "+91 98765 12003", "Industrial", 12540, 13010, 470, 5605, false}};
        for (auto &customer : customers)
            recalculate(customer);
    }

    void loadTariff()
    {
        std::ifstream input(kTariffFile);
        if (!(input >> tariff.domestic >> tariff.commercial >> tariff.industrial))
            tariff = Tariff{};
    }

    void saveTariff()
    {
        std::ofstream output(kTariffFile, std::ios::trunc);
        output << tariff.domestic << ' ' << tariff.commercial << ' ' << tariff.industrial << '\n';
    }

    void loadCustomers()
    {
        std::ifstream input(kCustomerFile);
        customers.clear();
        std::string line;
        while (std::getline(input, line))
        {
            const auto parts = split(line, '|');
            if (parts.size() != 10)
                continue;

            Customer customer;
            customer.id = std::stoi(parts[0]);
            customer.name = parts[1];
            customer.address = parts[2];
            customer.contact = parts[3];
            customer.type = parts[4];
            customer.previousReading = std::stod(parts[5]);
            customer.currentReading = std::stod(parts[6]);
            customer.unitsConsumed = std::stod(parts[7]);
            customer.billAmount = std::stod(parts[8]);
            customer.isPaid = parts[9] == "1";
            customers.push_back(customer);
        }

        if (customers.empty())
            seedCustomers();
    }

    void saveCustomers()
    {
        std::ofstream output(kCustomerFile, std::ios::trunc);
        output << std::fixed << std::setprecision(2);
        for (const auto &customer : customers)
        {
            output << customer.id << '|'
                   << customer.name << '|'
                   << customer.address << '|'
                   << customer.contact << '|'
                   << customer.type << '|'
                   << customer.previousReading << '|'
                   << customer.currentReading << '|'
                   << customer.unitsConsumed << '|'
                   << customer.billAmount << '|'
                   << (customer.isPaid ? "1" : "0") << '\n';
        }
    }

    int nextCustomerId()
    {
        int next = 1001;
        for (const auto &customer : customers)
            next = std::max(next, customer.id + 1);
        return next;
    }

    Customer *findCustomer(int id)
    {
        auto it = std::find_if(customers.begin(), customers.end(), [id](const Customer &customer)
                               { return customer.id == id; });
        return it == customers.end() ? nullptr : &(*it);
    }

    std::string billJson(double previous, double current, const std::string &type)
    {
        const Bill bill = calculateBill(previous, current, type);
        std::ostringstream out;
        out << std::fixed << std::setprecision(2)
            << "{\"units\":" << bill.units
            << ",\"energy\":" << bill.energy
            << ",\"fixed\":" << bill.fixed
            << ",\"tax\":" << bill.tax
            << ",\"total\":" << bill.total << '}';
        return out.str();
    }

    std::string customerJson(const Customer &customer)
    {
        std::ostringstream out;
        out << std::fixed << std::setprecision(2)
            << "{\"id\":" << customer.id
            << ",\"name\":\"" << jsonEscape(customer.name)
            << "\",\"address\":\"" << jsonEscape(customer.address)
            << "\",\"contact\":\"" << jsonEscape(customer.contact)
            << "\",\"type\":\"" << jsonEscape(customer.type)
            << "\",\"previousReading\":" << customer.previousReading
            << ",\"currentReading\":" << customer.currentReading
            << ",\"unitsConsumed\":" << customer.unitsConsumed
            << ",\"billAmount\":" << customer.billAmount
            << ",\"isPaid\":" << (customer.isPaid ? "true" : "false")
            << ",\"bill\":" << billJson(customer.previousReading, customer.currentReading, customer.type)
            << '}';
        return out.str();
    }

    std::string customersJson()
    {
        std::ostringstream out;
        out << '[';
        for (std::size_t i = 0; i < customers.size(); ++i)
        {
            if (i)
                out << ',';
            out << customerJson(customers[i]);
        }
        out << ']';
        return out.str();
    }

    std::string tariffJson()
    {
        std::ostringstream out;
        out << std::fixed << std::setprecision(2)
            << "{\"Domestic\":" << tariff.domestic
            << ",\"Commercial\":" << tariff.commercial
            << ",\"Industrial\":" << tariff.industrial
            << ",\"fixedCharge\":" << kFixedCharge
            << ",\"taxRate\":" << kTaxRate << '}';
        return out.str();
    }

    std::string reportJson()
    {
        double revenue = 0.0;
        double pendingAmount = 0.0;
        double totalUnits = 0.0;
        int paid = 0;
        int pending = 0;
        std::map<std::string, double> segmentTotals{{"Domestic", 0}, {"Commercial", 0}, {"Industrial", 0}};

        for (const auto &customer : customers)
        {
            totalUnits += customer.unitsConsumed;
            segmentTotals[customer.type] += customer.billAmount;
            if (customer.isPaid)
            {
                revenue += customer.billAmount;
                ++paid;
            }
            else
            {
                pendingAmount += customer.billAmount;
                ++pending;
            }
        }

        std::string topSegment = "Domestic";
        for (const auto &entry : segmentTotals)
        {
            if (entry.second > segmentTotals[topSegment])
                topSegment = entry.first;
        }

        std::ostringstream out;
        out << std::fixed << std::setprecision(2)
            << "{\"revenue\":" << revenue
            << ",\"pendingAmount\":" << pendingAmount
            << ",\"paidCount\":" << paid
            << ",\"pendingCount\":" << pending
            << ",\"customerCount\":" << customers.size()
            << ",\"averageUnits\":" << (customers.empty() ? 0.0 : totalUnits / customers.size())
            << ",\"topSegment\":\"" << topSegment << "\""
            << ",\"segmentTotals\":{\"Domestic\":" << segmentTotals["Domestic"]
            << ",\"Commercial\":" << segmentTotals["Commercial"]
            << ",\"Industrial\":" << segmentTotals["Industrial"] << "}}";
        return out.str();
    }

    std::string appStateJson()
    {
        return "{\"customers\":" + customersJson() + ",\"tariffs\":" + tariffJson() + ",\"report\":" + reportJson() + "}";
    }

    std::string response(int status, const std::string &statusText, const std::string &type, const std::string &body)
    {
        std::ostringstream out;
        out << "HTTP/1.1 " << status << ' ' << statusText << "\r\n"
            << "Content-Type: " << type << "\r\n"
            << "X-Content-Type-Options: nosniff\r\n"
            << "X-Frame-Options: DENY\r\n"
            << "Referrer-Policy: no-referrer\r\n"
            << "Content-Security-Policy: default-src 'self'; script-src 'self'; style-src 'self' 'unsafe-inline' https://fonts.googleapis.com; font-src 'self' https://fonts.gstatic.com; img-src 'self' data:; connect-src 'self'; frame-ancestors 'none'\r\n"
            << "Content-Length: " << body.size() << "\r\n"
            << "Connection: close\r\n\r\n"
            << body;
        return out.str();
    }

    std::string jsonResponse(const std::string &body)
    {
        return response(200, "OK", "application/json", body);
    }

    std::string errorResponse(int status, const std::string &message)
    {
        const std::string text = status == 404 ? "Not Found" : status == 400 ? "Bad Request"
                                                       : status == 413   ? "Payload Too Large"
                                                                             : "Server Error";
        return response(status, text, "application/json", "{\"error\":\"" + jsonEscape(message) + "\"}");
    }

    std::string mimeFor(const std::string &path)
    {
        if (path.size() >= 4 && path.substr(path.size() - 4) == ".css")
            return "text/css; charset=utf-8";
        if (path.size() >= 3 && path.substr(path.size() - 3) == ".js")
            return "application/javascript; charset=utf-8";
        if (path.size() >= 5 && path.substr(path.size() - 5) == ".html")
            return "text/html; charset=utf-8";
        return "text/plain; charset=utf-8";
    }

    std::string readFile(const std::string &fileName)
    {
        std::ifstream input(fileName, std::ios::binary);
        if (!input)
            return "";
        std::ostringstream buffer;
        buffer << input.rdbuf();
        return buffer.str();
    }

    bool parseRequest(const std::string &raw, HttpRequest &request)
    {
        const auto headerEnd = raw.find("\r\n\r\n");
        if (headerEnd == std::string::npos)
            return false;

        std::istringstream headers(raw.substr(0, headerEnd));
        std::string requestLine;
        if (!std::getline(headers, requestLine))
            return false;
        if (!requestLine.empty() && requestLine.back() == '\r')
            requestLine.pop_back();

        std::istringstream firstLine(requestLine);
        firstLine >> request.method >> request.path;
        if (request.method.empty() || request.path.empty())
            return false;

        std::string line;
        while (std::getline(headers, line))
        {
            if (!line.empty() && line.back() == '\r')
                line.pop_back();
            const auto colon = line.find(':');
            if (colon == std::string::npos)
                continue;
            request.headers[lower(trim(line.substr(0, colon)))] = trim(line.substr(colon + 1));
        }

        request.body = raw.substr(headerEnd + 4);
        const auto query = request.path.find('?');
        if (query != std::string::npos)
            request.path = request.path.substr(0, query);
        return true;
    }

    std::string handleApi(const HttpRequest &request)
    {
        if (request.method == "OPTIONS")
            return response(204, "No Content", "text/plain", "");

        if (request.method == "GET" && request.path == "/api/state")
            return jsonResponse(appStateJson());

        if (request.method == "GET" && request.path == "/api/customers")
            return jsonResponse(customersJson());

        if (request.method == "GET" && request.path == "/api/tariffs")
            return jsonResponse(tariffJson());

        if (request.method == "GET" && request.path == "/api/report")
            return jsonResponse(reportJson());

        if (request.method == "POST" && request.path == "/api/customers")
        {
            Customer customer;
            double previous = 0.0;
            double current = 0.0;
            if (!extractString(request.body, "name", customer.name) ||
                !extractString(request.body, "address", customer.address) ||
                !extractString(request.body, "contact", customer.contact) ||
                !extractString(request.body, "type", customer.type) ||
                !extractDouble(request.body, "previousReading", previous) ||
                !extractDouble(request.body, "currentReading", current))
                return errorResponse(400, "Missing customer fields.");

            if (trim(customer.name).empty() || trim(customer.address).empty() || trim(customer.contact).empty())
                return errorResponse(400, "Customer name, address, and contact are required.");
            if (customer.type != "Domestic" && customer.type != "Commercial" && customer.type != "Industrial")
                return errorResponse(400, "Invalid customer type.");
            if (previous < 0 || current < previous)
                return errorResponse(400, "Current reading must be greater than or equal to previous reading.");

            customer.id = nextCustomerId();
            customer.previousReading = previous;
            customer.currentReading = current;
            customer.isPaid = false;
            recalculate(customer);
            customers.insert(customers.begin(), customer);
            saveCustomers();
            return jsonResponse("{\"message\":\"Customer created.\",\"state\":" + appStateJson() + "}");
        }

        if (request.method == "POST" && request.path == "/api/bills/generate")
        {
            int id = 0;
            double current = 0.0;
            if (!extractInt(request.body, "customerId", id) || !extractDouble(request.body, "currentReading", current))
                return errorResponse(400, "Customer id and current reading are required.");

            Customer *customer = findCustomer(id);
            if (!customer)
                return errorResponse(404, "Customer not found.");
            if (current < customer->currentReading)
                return errorResponse(400, "Current reading cannot be lower than the previous stored reading.");

            customer->previousReading = customer->currentReading;
            customer->currentReading = current;
            customer->isPaid = false;
            recalculate(*customer);
            saveCustomers();
            return jsonResponse("{\"message\":\"Bill generated.\",\"customer\":" + customerJson(*customer) + ",\"state\":" + appStateJson() + "}");
        }

        if (request.method == "POST" && request.path == "/api/payments")
        {
            int id = 0;
            bool paid = false;
            if (!extractInt(request.body, "customerId", id) || !extractBool(request.body, "isPaid", paid))
                return errorResponse(400, "Customer id and payment status are required.");

            Customer *customer = findCustomer(id);
            if (!customer)
                return errorResponse(404, "Customer not found.");

            customer->isPaid = paid;
            saveCustomers();
            return jsonResponse("{\"message\":\"Payment status updated.\",\"state\":" + appStateJson() + "}");
        }

        if (request.method == "POST" && request.path == "/api/tariffs")
        {
            double domestic = 0.0;
            double commercial = 0.0;
            double industrial = 0.0;
            if (!extractDouble(request.body, "Domestic", domestic) ||
                !extractDouble(request.body, "Commercial", commercial) ||
                !extractDouble(request.body, "Industrial", industrial))
                return errorResponse(400, "All three tariff rates are required.");
            if (domestic <= 0 || commercial <= 0 || industrial <= 0)
                return errorResponse(400, "Tariff rates must be positive.");

            tariff.domestic = domestic;
            tariff.commercial = commercial;
            tariff.industrial = industrial;
            for (auto &customer : customers)
                recalculate(customer);
            saveTariff();
            saveCustomers();
            return jsonResponse("{\"message\":\"Tariffs updated and bills recalculated.\",\"state\":" + appStateJson() + "}");
        }

        const std::string customerPrefix = "/api/customers/";
        if (request.method == "DELETE" && request.path.rfind(customerPrefix, 0) == 0)
        {
            int id = 0;
            try
            {
                const std::string rawId = request.path.substr(customerPrefix.size());
                if (rawId.empty() || !std::all_of(rawId.begin(), rawId.end(), [](unsigned char ch)
                                                  { return std::isdigit(ch) != 0; }))
                    return errorResponse(400, "Invalid customer id.");
                id = std::stoi(rawId);
            }
            catch (...)
            {
                return errorResponse(400, "Invalid customer id.");
            }

            const auto before = customers.size();
            customers.erase(std::remove_if(customers.begin(), customers.end(), [id](const Customer &customer)
                                           { return customer.id == id; }),
                            customers.end());
            if (customers.size() == before)
                return errorResponse(404, "Customer not found.");
            saveCustomers();
            return jsonResponse("{\"message\":\"Customer deleted.\",\"state\":" + appStateJson() + "}");
        }

        return errorResponse(404, "Unknown API endpoint.");
    }

    std::string handleStatic(const HttpRequest &request)
    {
        if (request.method != "GET")
            return errorResponse(404, "Route not found.");

        std::string fileName;
        if (request.path == "/" || request.path == "/index.html")
            fileName = "index.html";
        else if (request.path == "/styles.css")
            fileName = "styles.css";
        else if (request.path == "/script.js")
            fileName = "script.js";
        else
            return errorResponse(404, "File not found.");

        const std::string body = readFile(fileName);
        if (body.empty())
            return errorResponse(404, "File not found.");
        return response(200, "OK", mimeFor(fileName), body);
    }

    std::string handleRequest(const HttpRequest &request)
    {
        if (request.path.rfind("/api/", 0) == 0)
            return handleApi(request);
        return handleStatic(request);
    }

    void closeSocket(Socket socket)
    {
#ifdef _WIN32
        closesocket(socket);
#else
        close(socket);
#endif
    }

    std::string receiveRequest(Socket client)
    {
        std::string raw;
        char buffer[4096];
        int expectedLength = -1;

        while (true)
        {
            const int received = recv(client, buffer, sizeof(buffer), 0);
            if (received <= 0)
                break;
            raw.append(buffer, buffer + received);
            if (raw.size() > kMaxRequestBytes)
                break;

            const auto headerEnd = raw.find("\r\n\r\n");
            if (headerEnd != std::string::npos && expectedLength < 0)
            {
                HttpRequest partial;
                if (parseRequest(raw, partial))
                {
                    const auto found = partial.headers.find("content-length");
                    try
                    {
                        expectedLength = found == partial.headers.end() ? 0 : std::stoi(found->second);
                    }
                    catch (...)
                    {
                        expectedLength = static_cast<int>(kMaxRequestBytes) + 1;
                    }
                    if (expectedLength > static_cast<int>(kMaxRequestBytes))
                        break;
                }
            }

            if (expectedLength >= 0)
            {
                const auto bodyStart = raw.find("\r\n\r\n") + 4;
                if (raw.size() >= bodyStart + static_cast<std::size_t>(expectedLength))
                    break;
            }
        }

        return raw;
    }

    bool initializeSockets()
    {
#ifdef _WIN32
        WSADATA data;
        return WSAStartup(MAKEWORD(2, 2), &data) == 0;
#else
        return true;
#endif
    }

    void shutdownSockets()
    {
#ifdef _WIN32
        WSACleanup();
#endif
    }
}

int main()
{
    const int port = configuredPort();
    loadTariff();
    loadCustomers();
    saveTariff();
    saveCustomers();

    if (!initializeSockets())
    {
        std::cerr << "Unable to initialize sockets.\n";
        return 1;
    }

    Socket server = socket(AF_INET, SOCK_STREAM, 0);
    if (server == INVALID_SOCKET)
    {
        std::cerr << "Unable to create server socket.\n";
        shutdownSockets();
        return 1;
    }

    int option = 1;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(&option), sizeof(option));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port);

    if (bind(server, reinterpret_cast<sockaddr *>(&address), sizeof(address)) == SOCKET_ERROR)
    {
        std::cerr << "Unable to bind port " << port << ". Close any running server and try again.\n";
        closeSocket(server);
        shutdownSockets();
        return 1;
    }

    if (listen(server, 16) == SOCKET_ERROR)
    {
        std::cerr << "Unable to listen for connections.\n";
        closeSocket(server);
        shutdownSockets();
        return 1;
    }

    std::cout << "Electricity Billing service running on port " << port << "\n";
    std::cout << "Open the URL above in your browser. Press Ctrl+C to stop.\n";

    while (true)
    {
        Socket client = accept(server, nullptr, nullptr);
        if (client == INVALID_SOCKET)
            continue;

        std::string output;
        try
        {
            const std::string raw = receiveRequest(client);
            HttpRequest request;
            output = raw.size() > kMaxRequestBytes
                         ? errorResponse(413, "Request is too large.")
                         : parseRequest(raw, request)
                               ? handleRequest(request)
                               : errorResponse(400, "Invalid HTTP request.");
        }
        catch (const std::exception &error)
        {
            output = errorResponse(500, error.what());
        }
        catch (...)
        {
            output = errorResponse(500, "Unexpected server error.");
        }

        if (!output.empty())
            send(client, output.c_str(), static_cast<int>(output.size()), 0);
        closeSocket(client);
    }

    closeSocket(server);
    shutdownSockets();
    return 0;
}
