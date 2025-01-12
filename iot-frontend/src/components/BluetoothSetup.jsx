import React, { useState, useEffect } from 'react';
import axios from 'axios';

const BluetoothSetup = () => {
    const [networks, setNetworks] = useState([]); // Available WiFi networks
    const [selectedNetwork, setSelectedNetwork] = useState(''); // Selected WiFi network
    const [password, setPassword] = useState(''); // Password fetched from the backend
    const [log, setLog] = useState(''); // Log messages for user feedback
    const [device, setDevice] = useState(null); // Connected Bluetooth device
    const [jsonPayload, setJsonPayload] = useState(''); // JSON payload to display

    // Fetch available WiFi networks from backend
    useEffect(() => {
        const fetchNetworks = async () => {
            try {
                const response = await axios.get('http://localhost:4000/wifi', {
                    headers: { Authorization: `Bearer ${localStorage.getItem('token')}` },
                });
                setNetworks(response.data);
            } catch (error) {
                console.error('Failed to fetch WiFi networks:', error);
                setLog('Failed to fetch WiFi networks.');
            }
        };
        fetchNetworks();
    }, []);

    // Fetch password for the selected network
    const fetchPassword = async (ssid) => {
        try {
            const response = await axios.get(`http://localhost:4000/wifi/${ssid}`, {
                headers: { Authorization: `Bearer ${localStorage.getItem('token')}` },
            });
            setPassword(response.data.password); // Set the fetched password
            setLog(`Password fetched for network: ${ssid}`);
        } catch (error) {
            console.error('Failed to fetch password:', error);
            setLog('Failed to fetch password.');
        }
    };

    // Handle network selection
    const handleNetworkSelection = (e) => {
        const ssid = e.target.value;
        setSelectedNetwork(ssid);
        setPassword(''); // Clear the password until it's fetched
        if (ssid) {
            fetchPassword(ssid);
        }
    };

    // Connect to ESP32 via Bluetooth
    const connectToDevice = async () => {
        try {
            setLog('Requesting Bluetooth device...');
            const selectedDevice = await navigator.bluetooth.requestDevice({
                acceptAllDevices: true, // Allows all Bluetooth devices
                optionalServices: ['battery_service'], // Replace with your ESP32 service UUID
            });

            setDevice(selectedDevice);
            setLog(`Connected to: ${selectedDevice.name}`);
        } catch (error) {
            console.error('Failed to connect to device:', error);
            setLog(`Failed to connect: ${error.message}`);
        }
    };

    // Send WiFi credentials to the ESP32
    const sendWiFiCredentials = async () => {
        if (!device) {
            setLog('No Bluetooth device connected.');
            return;
        }

        const payload = {
            ssid: selectedNetwork,
            password: password,
        };

        setJsonPayload(JSON.stringify(payload, null, 2)); // Display the JSON payload

        try {
            const server = await device.gatt.connect();
            const service = await server.getPrimaryService('your-service-uuid'); // Replace with your ESP32's service UUID
            const characteristic = await service.getCharacteristic('your-characteristic-uuid'); // Replace with your ESP32's characteristic UUID

            const credentials = new TextEncoder().encode(JSON.stringify(payload));
            await characteristic.writeValue(credentials);

            setLog(`WiFi credentials sent to ESP32: ${JSON.stringify(payload)}`);
        } catch (error) {
            console.error('Failed to send WiFi credentials:', error);
            setLog(`Failed to send credentials: ${error.message}`);
        }
    };

    return (
        <div className="p-6 bg-gray-100 min-h-screen flex flex-col items-center">
            <h1 className="text-3xl font-bold mb-6">Bluetooth Setup</h1>

            {/* Connect to Bluetooth Device */}
            <button
                onClick={connectToDevice}
                className="mb-6 bg-blue-500 text-white font-semibold py-2 px-4 rounded hover:bg-blue-600 transition"
            >
                Connect to Bluetooth Device
            </button>

            {device && (
                <div className="w-full max-w-md bg-white p-6 rounded shadow-md">
                    {/* Show connected device */}
                    <p className="text-lg font-semibold mb-4">
                        Currently connected to: <span className="text-green-500">{device.name}</span>
                    </p>

                    {/* Select WiFi Network */}
                    <h2 className="text-xl font-bold mb-4">Select WiFi Network</h2>
                    <select
                        value={selectedNetwork}
                        onChange={handleNetworkSelection}
                        className="w-full p-2 border border-gray-300 rounded mb-4"
                    >
                        <option value="">Select a network</option>
                        {networks.map((network) => (
                            <option key={network.id} value={network.ssid}>
                                {network.ssid}
                            </option>
                        ))}
                    </select>

                    {/* Display Fetched Password */}
                    {password && (
                        <p className="text-sm text-gray-700 mb-4">
                            <strong>Password:</strong> {password}
                        </p>
                    )}

                    {/* Send WiFi Credentials */}
                    <button
                        onClick={sendWiFiCredentials}
                        disabled={!selectedNetwork || !password}
                        className={`w-full py-2 px-4 rounded font-semibold transition ${
                            selectedNetwork && password
                                ? 'bg-green-500 text-white hover:bg-green-600'
                                : 'bg-gray-300 text-gray-600 cursor-not-allowed'
                        }`}
                    >
                        Send WiFi to ESP32
                    </button>
                </div>
            )}

            {/* JSON Payload Display */}
            {jsonPayload && (
                <div className="w-full max-w-md mt-6 p-4 bg-gray-100 rounded shadow-md">
                    <h3 className="text-lg font-semibold mb-2">JSON Payload</h3>
                    <pre className="bg-gray-800 text-white p-4 rounded overflow-x-auto text-sm">
                        {jsonPayload}
                    </pre>
                </div>
            )}

            {/* Log Messages */}
            <div className="w-full max-w-md mt-6 p-4 bg-gray-200 rounded">
                <p className="text-sm text-gray-700">{log}</p>
            </div>
        </div>
    );
};

export default BluetoothSetup;
