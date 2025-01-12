import React, { useState } from 'react';
import axios from 'axios';
import { Link, useNavigate } from 'react-router-dom';

const LoginForm = ({ onLogin }) => {
    const navigate = useNavigate();
    const [email, setEmail] = useState('');
    const [password, setPassword] = useState('');
    const [status, setStatus] = useState('');

    const handleLogin = async (e) => {
        e.preventDefault();
    
        try {
            const response = await axios.post('http://localhost:4000/login', { email, password });
            localStorage.setItem('token', response.data.token);
            localStorage.setItem('email', email);
            onLogin(response.data.token, email); // Pass token and email to context
            navigate('/dashboard');
        } catch (err) {
            console.error('Login failed:', err);
            setStatus('Invalid credentials');
        }
    };
    

    return (
        <form onSubmit={handleLogin} className="flex flex-col gap-4 p-6 bg-white rounded-lg shadow-md max-w-md mx-auto mt-10">
            <h2 className="text-2xl font-bold text-center text-primary">Login</h2>
            <input
                type="email"
                placeholder="Email"
                value={email}
                onChange={(e) => setEmail(e.target.value)}
                required
                className="p-2 border rounded-md focus:outline-none focus:ring focus:ring-secondary"
            />
            <input
                type="password"
                placeholder="Password"
                value={password}
                onChange={(e) => setPassword(e.target.value)}
                required
                className="p-2 border rounded-md focus:outline-none focus:ring focus:ring-secondary"
            />
            <button type="submit" className="bg-primary text-white py-2 px-4 rounded-md hover:bg-secondary transition">
                Login
            </button>
            {status && <p className="text-center mt-4 text-red-500">{status}</p>}
            <p className="text-center mt-4">
                Don't have an account?{' '}
                <Link to="/register" className="text-primary hover:underline">
                    Register here
                </Link>
            </p>
        </form>
    );
};

export default LoginForm;
