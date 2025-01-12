import React, { useState, useEffect, useContext } from 'react';
import WiFiList from '../components/WiFiList';
import AddWiFiForm from '../components/AddWiFiForm';
import DeviceList from '../components/DeviceList';
import AddDeviceForm from '../components/AddDeviceForm';
import { AuthContext } from '../AuthContext';
import axios from 'axios';

const DashboardPage = () => {
    const { token } = useContext(AuthContext);
    const [showAddWiFiForm, setShowAddWiFiForm] = useState(false);
    const [showAddDeviceForm, setShowAddDeviceForm] = useState(false);
    const [wifiNetworks, setWifiNetworks] = useState([]);
    const [devices, setDevices] = useState([]);

    useEffect(() => {
        const fetchWiFiNetworks = async () => {
            try {
                const response = await axios.get('http://localhost:4000/wifi', {
                    headers: { Authorization: `Bearer ${token}` },
                });
                setWifiNetworks(response.data);
            } catch (err) {
                console.error('Failed to fetch WiFi networks:', err);
            }
        };

        const fetchDevices = async () => {
            try {
                const response = await axios.get('http://localhost:4000/devices', {
                    headers: { Authorization: `Bearer ${token}` },
                });
                setDevices(response.data);
            } catch (err) {
                console.error('Failed to fetch devices:', err);
            }
        };

        fetchWiFiNetworks();
        fetchDevices();
    }, [token]);

    const toggleAddWiFiForm = () => {
        setShowAddWiFiForm((prev) => !prev);
    };

    const toggleAddDeviceForm = () => {
        setShowAddDeviceForm((prev) => !prev);
    };

    const handleWiFiAdded = (newNetwork) => {
        setWifiNetworks((prev) => [...prev, newNetwork]);
        setShowAddWiFiForm(false);
    };

    const handleDeviceAdded = (newDevice) => {
        setDevices((prev) => [...prev, newDevice]);
        setShowAddDeviceForm(false);
    };

    return (
        <div className="p-6 bg-background min-h-screen mt-16">
            <h1 className="text-3xl font-bold mb-6 text-primary text-center">Dashboard</h1>
            <div className="flex flex-col items-center">
                <button
                    onClick={toggleAddWiFiForm}
                    className="mb-4 bg-primary text-white py-2 px-4 rounded-md hover:bg-secondary transition"
                >
                    {showAddWiFiForm ? 'Cancel' : 'Add WiFi Network'}
                </button>
                {showAddWiFiForm && <AddWiFiForm onAddWiFi={handleWiFiAdded} />}
                <button
                    onClick={toggleAddDeviceForm}
                    className="mb-4 bg-secondary text-white py-2 px-4 rounded-md hover:bg-primary transition"
                >
                    {showAddDeviceForm ? 'Cancel' : 'Add Device'}
                </button>
                {showAddDeviceForm && <AddDeviceForm onAddDevice={handleDeviceAdded} />}
            </div>
            <div className="flex flex-col items-center mt-6">
                <WiFiList networks={wifiNetworks} />
            </div>
            <div className="flex flex-col items-center mt-6">
                <DeviceList devices={devices} />
            </div>
        </div>
    );
};

export default DashboardPage;
