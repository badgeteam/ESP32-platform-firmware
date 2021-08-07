#ifndef WILDCARD_CERT_H
#define WILDCARD_CERT_H

#define NUM_PINNED_CERTS 4

typedef struct {
  const unsigned int data_len;
  const unsigned char* data;
} cert_t;

extern const cert_t pinned_certificates[];

#endif
