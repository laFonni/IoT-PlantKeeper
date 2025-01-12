import React, { useState } from 'react';
import WiFiList from '../components/WiFiList';
import AddWiFiForm from '../components/AddWiFiForm';
import DeviceList from '../components/DeviceList';
import AddDeviceForm from '../components/AddDeviceForm';

const DashboardPage = () => {
    const [showAddWiFiForm, setShowAddWiFiForm] = useState(false);
    const [showAddDeviceForm, setShowAddDeviceForm] = useState(false);

    const toggleAddWiFiForm = () => {
        setShowAddWiFiForm((prev) => !prev);
    };

    const toggleAddDeviceForm = () => {
        setShowAddDeviceForm((prev) => !prev);
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
            {showAddWiFiForm && <AddWiFiForm />}
            <button
                onClick={toggleAddDeviceForm}
                className="mb-4 bg-green-500 text-white py-2 px-4 rounded-md hover:bg-green-600 transition"
            >
                {showAddDeviceForm ? 'Cancel' : 'Add Device'}
            </button>
            {showAddDeviceForm && <AddDeviceForm />}
            <WiFiList />
            <DeviceList />
        </div>
    );
};

export default DashboardPage;
