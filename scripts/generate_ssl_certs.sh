#!/bin/bash

set -e

# Create certificates directory
CERT_DIR="$(pwd)/certs"
mkdir -p "$CERT_DIR"

cd "$CERT_DIR"

echo "Generating SSL certificates for NPRPC SSL WebSocket testing..."

# Generate private key
openssl genrsa -out server.key 2048

# Generate certificate signing request
openssl req -new -key server.key -out server.csr -subj "/C=US/ST=Test/L=Test/O=NPRPC/OU=Testing/CN=localhost"

# Generate self-signed certificate valid for 365 days
openssl x509 -req -days 365 -in server.csr -signkey server.key -out server.crt

# Generate DH parameters (optional, for stronger security)
openssl dhparam -out dhparam.pem 2048

# Set appropriate permissions
chmod 600 server.key
chmod 644 server.crt dhparam.pem

echo "SSL certificates generated successfully:"
echo "  Private key: $CERT_DIR/server.key"
echo "  Certificate: $CERT_DIR/server.crt"
echo "  DH params:   $CERT_DIR/dhparam.pem"
echo ""
echo "To use these certificates with NPRPC, call:"
echo "  rpc_builder.enable_ssl_server("
echo "    \"$CERT_DIR/server.crt\","
echo "    \"$CERT_DIR/server.key\","
echo "    \"$CERT_DIR/dhparam.pem\")"

# Clean up CSR file
rm server.csr

echo ""
echo "Certificates are ready for SSL WebSocket testing!"
