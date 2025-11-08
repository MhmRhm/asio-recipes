# Generating TLS Certificate, Private Key, and DH Parameters

This guide explains how to generate a self-signed TLS certificate (`server.crt`),
a private key (`server.key`), and Diffie–Hellman parameters (`dhparams.pem`)
using OpenSSL.

## 1. Generate `server.key` and `server.crt`

The following command creates:

* a new **Ed25519 private key** (`server.key`)
* a **self-signed certificate** (`server.crt`)
* a **Subject Alternative Name (SAN)** entry for an IP address
* an **encrypted private key** (OpenSSL will prompt for a password unless
`-passout` is used)

Replace the IP address with the one your server actually uses.

```sh
openssl req -x509 -newkey ed25519 \
  -keyout server.key \
  -out server.crt \
  -subj "/CN=192.168.64.2" \
  -addext "subjectAltName=IP:192.168.64.2"
```

### Optional: Set the password non-interactively

```sh
-passout pass:YourPasswordHere
```

Example:

```sh
openssl req -x509 -newkey ed25519 \
  -keyout server.key \
  -out server.crt \
  -subj "/CN=192.168.64.2" \
  -addext "subjectAltName=IP:192.168.64.2" \
  -passout pass:changeme123
```

## 2. Generate `dhparams.pem`

DH parameters are used by certain TLS cipher suites for perfect forward secrecy.
They are not tied to Ed25519 but may be required by your server (for example,
Nginx when using non-ECDHE ciphers).

Generate a 2048-bit DH parameter file:

```sh
openssl dhparam -out dhparams.pem 2048
```

For stronger (but slower-to-generate) parameters:

```sh
openssl dhparam -out dhparams.pem 4096
```

## Output Files

After running the commands, you should have:

* `server.key` – your private key (encrypted unless `-noenc` was used)
* `server.crt` – your self-signed certificate
* `dhparams.pem` – your Diffie–Hellman parameters

## Notes

* If you need multiple SAN entries (IP + domain names), you can include them
like this:

  ```sh
  -addext "subjectAltName=IP:192.168.64.2,DNS:example.com,DNS:www.example.com"
  ```

* Browsers and modern TLS clients validate SAN, not the CN, so ensure SAN
entries match how clients connect to your server.
