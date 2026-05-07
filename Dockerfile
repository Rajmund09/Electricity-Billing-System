# Use a lightweight Linux image with GCC
FROM gcc:latest

# Set the working directory
WORKDIR /app

# Copy all project files
COPY . .

# Compile the C++ backend for Linux
# Note: The code already has #ifdef _WIN32 blocks to handle POSIX sockets on Linux
RUN g++ backend_server.cpp -std=c++17 -O3 -o BillingBackend

# Use the PORT environment variable provided by the host
# Our code already calls std::getenv("PORT")
EXPOSE 8080

# Run the backend
CMD ["./BillingBackend"]
