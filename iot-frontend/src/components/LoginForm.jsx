import React, { useState } from 'react';
import axios from 'axios';
import { Link } from 'react-router-dom';

const LoginForm = ({ onLogin }) => {
    const [email, setEmail] = useState('');
    const [password, setPassword] = useState('');
    const [status, setStatus] = useState('');

    const handleLogin = async (e) => {
        e.preventDefault();

        try {
            const response = await axios.post('http://localhost:4000/login', { email, password });
            setStatus('Login successful!');
            onLogin(response.data.token); // Przekazanie tokena do LoginPage
        } catch (err) {
            console.error('Login error:', err.response || err.message); // Logowanie błędów
            setStatus(err.response?.data || 'Invalid credentials'); // Wyświetlanie błędu backendu, jeśli istnieje
        }
    };

    return (
        <form onSubmit={handleLogin} className="flex flex-col gap-4 p-6 bg-white rounded-md shadow-md">
            <h2 className="text-2xl font-bold text-center">Login</h2>
            <input
                type="email"
                placeholder="Email"
                value={email}
                onChange={(e) => setEmail(e.target.value)}
                required
                className="p-2 border rounded-md"
            />
            <input
                type="password"
                placeholder="Password"
                value={password}
                onChange={(e) => setPassword(e.target.value)}
                required
                className="p-2 border rounded-md"
            />
            <button type="submit" className="bg-blue-500 text-white py-2 px-4 rounded-md">
                Login
            </button>
            {status && <p className="text-center mt-4 text-red-500">{status}</p>}
            <p className="text-center mt-4">
                Don't have an account?{' '}
                <Link to="/register" className="text-blue-500 hover:underline">
                    Register here
                </Link>
            </p>
        </form>
    );
};

export default LoginForm;
