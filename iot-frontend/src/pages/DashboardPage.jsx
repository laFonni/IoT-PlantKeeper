import React, { useState } from 'react';
import { useNavigate } from 'react-router-dom';
import WiFiList from '../components/WiFiList';
import AddWiFiForm from '../components/AddWiFiForm';

const DashboardPage = ({ token }) => {
    const [showAddWiFiForm, setShowAddWiFiForm] = useState(false);
    const navigate = useNavigate(); // Use React Router's navigation hook

    const toggleAddWiFiForm = () => {
        setShowAddWiFiForm((prev) => !prev);
    };

    return (
        <div className="p-6">
            <h1 className="text-3xl font-bold mb-6">Dashboard</h1>

            {/* Add WiFi Network Toggle */}
            <button
                onClick={toggleAddWiFiForm}
                className="mb-4 bg-blue-500 text-white py-2 px-4 rounded-md hover:bg-blue-600 transition"
            >
                {showAddWiFiForm ? 'Cancel' : 'Add WiFi Network'}
            </button>
            {showAddWiFiForm && <AddWiFiForm token={token} />}

            {/* WiFi List */}
            <WiFiList token={token} />

            {/* Navigate to Bluetooth Setup */}
            <button
                onClick={() => navigate('/bluetooth-setup')}
                className="mt-6 bg-green-500 text-white py-2 px-4 rounded-md hover:bg-green-600 transition"
            >
                Setup Bluetooth
            </button>
        </div>
    );
};

export default DashboardPage;
