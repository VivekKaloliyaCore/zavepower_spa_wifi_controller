#ifndef WIFI_MODULE_H
#define WIFI_MODULE_H
#include <Arduino.h>


#include "../../src/config.h" // Default passwords and SSID

#ifndef WIFI_SSID
#warning "WIFI_SSID not defined, please define in config.h"
#define WIFI_SSID "spa"
#endif

#ifndef WIFI_PASSWORD
#warning "WIFI_PASSWORD not defined, please define in config.h"
#define WIFI_PASSWORD "password"
#endif

#ifndef GMT_OFFSET
#warning "GMT_OFFSET not defined, please define in config.h"
#define GMT_OFFSET -14400
#endif

#ifndef DAYLIGHT_OFFSET
#warning "DAYLIGHT_OFFSET not defined, please define in config.h"
#define DAYLIGHT_OFFSET 0
#endif

#define WIFI_CONNECT_TIMEOUT 10000

#define WIFI_STA_CONNECT_RETRIES 5

const long gmtOffset_sec = GMT_OFFSET;
const int daylightOffset_sec = DAYLIGHT_OFFSET;
extern char gatewayName[20];

// HTML for Wi-Fi selection
const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Wi-Fi Setup</title>
    <style>
        /* Overlay */

        body{
            margin: 0;
        }
        *{
            box-sizing: border-box;
            font-family: sans-serif;
        }
        .modal-overlay {
          position: fixed;
          top: 0;
          left: 0;
          width: 100%;
          height: 100%;
          background-color: rgba(0,0,0,0.5);
          display: none;
          justify-content: center;
          align-items: center;
          z-index: 1000;
        }
    
        /* Modal Box */
        .modal-box {
          background: #0d2628;
          color: #fff;
          padding: 20px 30px;
          border-radius: 10px;
          max-width: 400px;
          width: 90%;
          box-shadow: 0 5px 15px rgba(0,0,0,0.3);
          animation: fadeIn 0.3s ease-in-out;
        }
    
        .modal-header {
          font-size: 18px;
          margin-bottom: 10px;
          font-weight: bold;
        }
    
        .modal-input {
          width: 100%;
          padding: 8px;
          margin: 20px 0 40px;
          font-size: 16px;
          border: 1px solid #1cbcc7;
          color: #fff;
          border-radius: 8px;
          background-color: transparent;
          height: 46px;
          outline: none;
        }
        .modal-input:focus{
          box-shadow: 0 0 0 .25rem #1cbcc736;
        }
    
        .modal-buttons {
          display: flex;
          justify-content: flex-end;
          gap: 10px;
        }
    
        .modal-buttons button {
          padding: 8px 16px;
          border-radius: 5px;
          cursor: pointer;
          border: 1px solid #1cbcc7;
        }
    
        .btn-cancel {
          background-color: transparent;
          color: #1cbcc7;
        }
    
        .btn-submit {
          background-color: #1cbcc7;
          color: #fff;
        }
    
        @keyframes fadeIn {
          from { opacity: 0; transform: scale(0.9); }
          to { opacity: 1; transform: scale(1); }
        }

        /* wifi setup css */
        .wifi-setup-area{
            display: flex;
            align-items: flex-start;
            justify-content: center;
            height: 100vh;
            width: 100%;
            font-family: sans-serif;
            background-color: #26221f;
          }
          .wifi-inner{
            border-radius: 12px;
            box-shadow: 3px 3px 6px #00000033;
            background-color: #0d2628;
            padding: 20px 20px 15px;
            width: 450px;
            max-width: 95%;
            margin: auto;
            color: #fff;
            height: 87vh;
            max-height: 650px;
        }
        .wifi-title{
          margin-top: 0;
            margin-bottom: 26px;
            text-align: center;
        }
        .notify{
          padding: 10px 14px;
          text-transform: capitalize;
          font-weight: 600;
          border: 1px solid #0e950c;
          color: #0e950c;
          margin: 0 0 6px;
        }
      </style>

    <style>
        .loader {
            border: 4px solid #f3f3f3;
            border-top: 4px solid #3498db;
            border-radius: 50%;
            width: 30px;
            height: 30px;
            animation: spin 1s linear infinite;
            margin: 50px auto 20px;
            display: none;
        }

        @keyframes spin {
            0% { transform: rotate(0deg); }
            100% { transform: rotate(360deg); }
        }

        .inner-button {
            display: block;
            margin: 5px 0;
            padding: 8px 15px;
            background-color: #1cbcc7;
            color: white;
            border: none;
            cursor: pointer;
            border-radius: 5px;
        }

        /* button:hover {
            background-color: #217dbb;
        } */
         .dn{
            display: none;
         }
         .avai-ttl{
          margin-bottom: 0;
         }
         .wifi-list-wrap{
          height: 74%;
         }
         .wifi-list{
            padding: 0;
            list-style: none;
            max-height: 80%;
            overflow-y: auto;
            margin-bottom: 18px;
         }
         .search-wifi-btn{
            width: 100%;
            font-weight: 600;
            font-size: 16px;
            margin-bottom: 40px;
            padding: 8px 6px;
            border-radius: 4px;
            border: 1px solid #1cbcc7;
         }
         .search-wifi-btn:hover{
          background-color: transparent;
          color: #1cbcc7;
         }
         .wifi-list li{
          border-bottom: 1px solid #6060604d;
         }
         .wifi-list li:last-child{
          border-bottom: none;
         }
         .wifi-list li button{
          width: 100%;
          text-align: left;
          border-radius: 0;
          padding: 13px 8px;
          border: none;
          background-color: transparent;
          font-size: 18px;
          cursor: pointer;
          color: #fff;
         }
         .wifi-list li button:hover{
          background-color: #1cbcc724;
         }
         /* width */
          ::-webkit-scrollbar {
            width: 4px;
            height: 4px;
          }

          /* Track */
          ::-webkit-scrollbar-track {
            border-radius: 10px;
            background-color: transparent ;
          }

          /* Handle */
          ::-webkit-scrollbar-thumb {
            background: #1cbcc7;
            border-radius: 10px;
          }
    </style>
    <script>function scanNetworks() {
      let networkList = document.getElementById("networks");
      let statusMsg = document.getElementById("status");
      let loader = document.getElementById("loader");
  
      // Clear previous scan results & messages
      networkList.innerHTML = "";
      // statusMsg.textContent = "Scanning...";
      loader.style.display = "block"; // Show loading animation
              
      fetch('/scan')
          .then(response => response.json())
          .then(data => {
              loader.style.display = "none"; // Hide loader
              statusMsg.textContent = ""; // Clear scanning message
  
              // Create the title
              let title = document.createElement("h3");
              title.className = "avai-ttl";
              title.textContent = "Available Networks";
              networkList.appendChild(title);
  
              // Create the list container
              let ul = document.createElement("ul");
              ul.className = "wifi-list";
  
              data.forEach(network => {
                  let li = document.createElement("li");
                  let btn = document.createElement("button");
                  btn.textContent = network;
                  btn.onclick = function() { openModal(network); };
                  li.appendChild(btn);
                  ul.appendChild(li);
              });
  
              networkList.appendChild(ul);
          })
          .catch(error => {
              loader.style.display = "none"; // Hide loader on error
              statusMsg.className = "notify";
              statusMsg.textContent = "Error scanning networks!";
          });
  }
  
        function selectNetwork() {
            // let pass = prompt("Enter Wi-Fi Password for " + ssid + ":");
            let pass = document.getElementById("wifiPassword").value;
            let ssid = document.getElementById("wifiSSID").value;
            console.log("pass>>>",pass)
            console.log("ssid>>>",ssid)
            if (pass === null) return; // User canceled

            let statusMsg = document.getElementById("status");
            statusMsg.className = "notify";
            statusMsg.textContent = "Saving credential...";

           fetch('/connect?ssid=' + encodeURIComponent(ssid) + '&pass=' + encodeURIComponent(pass))
                .then(response => response.text())
                .then(data => {
                    statusMsg.textContent = data; // Show response (e.g., "Saved successfully")
                    window.location.href = "/wifiCheck"; // Redirect after success
                })
                .catch(error => {
                    statusMsg.textContent = "Saved successfully";
                    window.location.href = "/wifiCheck"; // Redirect even if fetch fails
                });
            closeModal();
        }
   
   </script>
</head>
<body>
    <div class="wifi-setup-area">
        <div class="wifi-inner">
            <h2 class="wifi-title">SpaPilot Wi-Fi Setup</h2>
            <button class="inner-button search-wifi-btn" type="button" onclick="scanNetworks()">Scan Wi-Fi</button>

            

            <div id="loader" class="loader"></div>
            <div id="networks" class="wifi-list-wrap"></div>
            <p id="status" ></p>
            <!-- <button onclick="openModal('my network')">Enter Wi-Fi Password</button> -->
        </div>    
    </div>



  <!-- Modal -->
  <div class="modal-overlay" id="customModal">
    <div class="modal-box">
      <div class="modal-header" id="enterPassHead">Enter Wi-Fi Password</div>
      <input type="password" id="wifiPassword" class="modal-input" placeholder="Wi-Fi Password" />
      <input type="text" id="wifiSSID" class="modal-input dn" placeholder="Wi-Fi Password" />
      <div class="modal-buttons">
        <button class="btn-cancel" onclick="closeModal()">Cancel</button>
        <button class="btn-submit" onclick="selectNetwork()">Submit</button>
      </div>
    </div>
  </div>









    <!-- <p> youpass: <span id="passText"></span></p> -->
  
   

<script>
    function openModal(ssid) {
        document.getElementById("enterPassHead").textContent = "Enter Wi-Fi Password for '" + ssid + "'"
        document.getElementById("wifiSSID").value = ssid;
        document.getElementById("customModal").style.display = "flex";
    }


    function closeModal() {
      document.getElementById("customModal").style.display = "none";
    }

    function submitPassword() {
      const password = document.getElementById("wifiPassword").value;
      console.log("Entered Wi-Fi Password:", password);

      document.getElementById("passText").textContent = password;

      
      closeModal();
    }
  </script>
      
</body>
</html>
)rawliteral";      

// const char htmlPage[] PROGMEM = R"rawliteral(
// <!DOCTYPE html>
// <html>
// <head>
//     <title>ESP32 Wi-Fi Setup</title>
//     <script>
//         function scanNetworks() {
//             fetch('/scan')
//                 .then(response => response.json())
//                 .then(data => {
//                     let networkList = document.getElementById("networks");
//                     networkList.innerHTML = "";
//                     data.forEach(network => {
//                         let option = document.createElement("option");
//                         option.value = network;
//                         option.textContent = network;
//                         networkList.appendChild(option);
//                     });
//                 });
//         }

//         function saveWiFi() {
//             let ssid = document.getElementById("networks").value;
//             let pass = document.getElementById("password").value;
//             fetch('/connect?ssid=' + ssid + '&pass=' + pass)
//                 .then(response => response.text())
//                 .then(alert);
//         }
//     </script>
// </head>
// <body>
//     <h2>ESP32 Wi-Fi Setup</h2>
//     <button onclick="scanNetworks()">Scan Wi-Fi</button>
//     <select id="networks"></select>
//     <input type="password" id="password" placeholder="Wi-Fi Password">
//     <button onclick="saveWiFi()">Connect</button>
// </body>
// </html>
// )rawliteral";

void wifiModuleSetup();
void wifiModuleLoop();
void notifyOfUpdateStarted();
void notifyOfUpdateEnded();
void wifiConnect();
void otaSetup();

String getStringTime();
struct tm getStructTime();

void wifiModuleEraseStaConfig(void);


bool wifiModuleCnnectToWiFiOneTime(void);
int32_t wifiModuleGetRSSI(void);
bool syncTimeFromIpApi(void);

extern bool restart_esp;
extern bool ap_configuration_on;

// #include <Preferences.h>
// Preferences preferences;

void syncWithNetworkTime(int hr, int min);

#endif