#include <pgmspace.h>

#define SECRET
#define THINGNAME "DDD3" // change this for new aws "thing" names "DDDx", used in table name for DB

const char WIFI_SSID[] = "redacted";
const char WIFI_PASSWORD[] = "redacted";
const char AWS_IOT_ENDPOINT[] = "redacted";

// Amazon Root CA 1 // Stays the same for all "things"
static const char AWS_CERT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
// Amazon Root CA 1 Cert info
-----END CERTIFICATE-----
)EOF";

// Device Certificate                                               //change this for new "thing"
static const char AWS_CERT_CRT[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----
// Device Certificate info
-----END CERTIFICATE-----
)KEY";

// Device Private Key                                               //change this for new "things"
static const char AWS_CERT_PRIVATE[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
// Device Private Key info
-----END RSA PRIVATE KEY-----
)KEY";
