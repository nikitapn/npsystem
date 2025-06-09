#!/bin/bash

set -e

DOMAIN_NAME=$1

if [ -z "$DOMAIN_NAME" ]; then
    echo "Usage: $0 <DOMAIN_NAME>"
    exit 1
fi

# Create certificates directory
CERT_DIR="$(pwd)/certs"
mkdir -p "$CERT_DIR"

cd "$CERT_DIR"

echo "Generating SSL certificates for NPRPC SSL WebSocket testing..."

# Generate private key
openssl genrsa -out ${DOMAIN_NAME}.key 2048

# Generate certificate signing request
#openssl req -new -key ${DOMAIN_NAME}.key -out ${DOMAIN_NAME}.csr \
#    -subj "/C=US/ST=Test/L=Test/O=NPRPC/OU=Testing/CN=*.${DOMAIN_NAME}" \
#    -addext "subjectAltName=DNS:${DOMAIN_NAME},DNS:www.${DOMAIN_NAME}"

# Generate self-signed certificate valid for 365 days
#openssl x509 -req -days 365 -in ${DOMAIN_NAME}.csr -signkey ${DOMAIN_NAME}.key -out ${DOMAIN_NAME}.crt

openssl req -x509 -key ${DOMAIN_NAME}.key -out ${DOMAIN_NAME}.crt \
    -sha256 -days 36500 -nodes \
    -subj "/C=US/ST=NY/O=NPRPC/OU=SiteName/CN=${DOMAIN_NAME}" \
    -addext "subjectAltName=DNS:${DOMAIN_NAME},DNS:www.${DOMAIN_NAME}"

# Generate DH parameters (optional, for stronger security)
# openssl dhparam -out dhparam.pem 2048

# Set appropriate permissions
chmod 600 ${DOMAIN_NAME}.key
chmod 644 ${DOMAIN_NAME}.crt dhparam.pem

echo "SSL certificates generated successfully:"
echo "  Private key: $CERT_DIR/${DOMAIN_NAME}.key"
echo "  Certificate: $CERT_DIR/${DOMAIN_NAME}.crt"
echo "  DH params:   $CERT_DIR/dhparam.pem"
echo ""
echo "To use these certificates with NPRPC, call:"
echo "  rpc_builder.enable_ssl_server("
echo "    \"$CERT_DIR/${DOMAIN_NAME}.crt\","
echo "    \"$CERT_DIR/${DOMAIN_NAME}.key\","
echo "    \"$CERT_DIR/dhparam.pem\")"

# Clean up CSR file
rm -f ${DOMAIN_NAME}.csr

echo ""
echo "Certificates are ready for SSL WebSocket testing!"
