

// Adjust the following values to match your needs
// -----------------------------------------------
#define   servername "fileserver"     // Set your ESP server's logical name here   e.g. if "myserver" then address is http://myserver.local/
IPAddress local_IP(192, 168, 1, 150); // Set your ESP server's fixed IP address here  Ex: "192.168.0.150" or "myserver.local" to access it 
IPAddress gateway(192, 168, 1, 1);    // Set your network Gateway usually your Router base address
IPAddress subnet(255, 255, 255, 0);   // Set your network sub-network mask here
IPAddress dns(192,168,1,1);           // Set your network DNS usually your Router base address


const char ssid_1[]     = "Livebox-1234";           // Replace these with real SSID and PASSWORD 
const char password_1[] = "abcde12345ABCDE";

const char ssid_2[]     = "your_SSID2";
const char password_2[] = "your_PASSWORD_for SSID2";

const char ssid_3[]     = "your_SSID3";
const char password_3[] = "your_PASSWORD_for SSID3";

const char ssid_4[]     = "your_SSID4";
const char password_4[] = "your_PASSWORD_for SSID4";
