import React, { useState } from 'react';
import axios from 'axios';
import { useNavigate } from 'react-router-dom';

const RegisterForm = () => {
  const navigate = useNavigate();
  const [email, setEmail] = useState('');
  const [password, setPassword] = useState('');
  const [confirmPassword, setConfirmPassword] = useState('');
  const [status, setStatus] = useState('');

  const handleRegister = async (e) => {
    e.preventDefault();

    if (password !== confirmPassword) {
      setStatus('Passwords do not match');
      return;
    }

    try {
      await axios.post('http://localhost:4000/register', { email, password });
      setStatus('Registration successful');
      navigate('/login'); // Redirect to login page
    } catch (err) {
      console.error('Registration failed:', err);
      setStatus('Registration failed');
    }
  };

  return (
    <form
      onSubmit={handleRegister}
      className="flex flex-col gap-4 p-8 bg-white rounded-lg shadow-lg max-w-md mx-auto mt-12"
    >
      <h2 className="text-3xl font-bold text-center text-primary">Register</h2>
      <input
        type="email"
        placeholder="Email"
        value={email}
        onChange={(e) => setEmail(e.target.value)}
        required
        className="p-3 border rounded-md focus:outline-none focus:ring focus:ring-secondary"
      />
      <input
        type="password"
        placeholder="Password"
        value={password}
        onChange={(e) => setPassword(e.target.value)}
        required
        className="p-3 border rounded-md focus:outline-none focus:ring focus:ring-secondary"
      />
      <input
        type="password"
        placeholder="Confirm Password"
        value={confirmPassword}
        onChange={(e) => setConfirmPassword(e.target.value)}
        required
        className="p-3 border rounded-md focus:outline-none focus:ring focus:ring-secondary"
      />
      <button
        type="submit"
        className="bg-primary text-white py-3 px-5 rounded-md hover:bg-secondary transition"
      >
        Register
      </button>
      {status && (
        <p className={`text-center mt-4 ${status.includes('successful') ? 'text-green-500' : 'text-red-500'}`}>
          {status}
        </p>
      )}
    </form>
  );
};

export default RegisterForm;
