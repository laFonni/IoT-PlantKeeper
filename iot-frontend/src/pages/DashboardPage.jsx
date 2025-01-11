import React, { useState } from 'react';
import WiFiList from '../components/WiFiList';
import AddWiFiForm from '../components/AddWiFiForm';
import DeviceList from '../components/DeviceList';
import AddDeviceForm from '../components/AddDeviceForm';

const DashboardPage = ({ token }) => {
    const [showAddWiFiForm, setShowAddWiFiForm] = useState(false);
    const [showAddDeviceForm, setShowAddDeviceForm] = useState(false);

    const toggleAddWiFiForm = () => {
        setShowAddWiFiForm((prev) => !prev);
    };

    const toggleAddDeviceForm = () => {
        setShowAddDeviceForm((prev) => !prev);
    };

    const handleAddDevice = (newDevice) => {
        console.log('Adding new device to state:', newDevice);
        // You can update the device list state here if needed
    };

    return (
        <div className="p-6">
            <h1 className="text-3xl font-bold mb-6">Dashboard</h1>
            <button
                onClick={toggleAddWiFiForm}
                className="mb-4 bg-blue-500 text-white py-2 px-4 rounded-md hover:bg-blue-600 transition"
            >
                {showAddWiFiForm ? 'Cancel' : 'Add WiFi Network'}
            </button>
            {showAddWiFiForm && <AddWiFiForm token={token} />}
            <button
                onClick={toggleAddDeviceForm}
                className="mb-4 bg-green-500 text-white py-2 px-4 rounded-md hover:bg-green-600 transition"
            >
                {showAddDeviceForm ? 'Cancel' : 'Add Device'}
            </button>
            {showAddDeviceForm && <AddDeviceForm token={token} onAddDevice={handleAddDevice} />}
            <WiFiList token={token} />
            <DeviceList token={token} />
        </div>
    );
};

export default DashboardPage;
