import React, { useState, useEffect, useContext } from 'react';
import { Link, useNavigate } from 'react-router-dom';
import { AuthContext } from '../AuthContext';
import { FaLeaf, FaSignOutAlt, FaTachometerAlt } from 'react-icons/fa';

const Navbar = () => {
  const { logout } = useContext(AuthContext);
  const [email, setEmail] = useState(null);
  const navigate = useNavigate();

  useEffect(() => {
    const savedEmail = localStorage.getItem('email');
    if (savedEmail) {
      setEmail(savedEmail);
    }
  }, []);

  const handleLogout = () => {
    logout();
    navigate('/dashboard');
  };

  return (
    <nav className="bg-gradient-to-r from-primary to-secondary shadow-lg fixed top-0 left-0 w-full z-50">
      <div className="max-w-6xl mx-auto px-4">
        <div className="flex justify-between items-center py-4">
          <Link to="/dashboard" className="flex items-center space-x-2">
            <FaLeaf className="text-white text-3xl" />
            <span className="text-white font-serif text-2xl">PlantKeeper</span>
          </Link>
          <div className="flex items-center space-x-6">
            <span className="text-white">{email}</span>
            <Link
              to="/dashboard"
              className="flex items-center space-x-1 text-white hover:text-accent transition"
            >
              <FaTachometerAlt />
              <span>Dashboard</span>
            </Link>
            <button
              onClick={handleLogout}
              className="flex items-center space-x-1 text-white hover:text-accent transition"
            >
              <FaSignOutAlt />
              <span>Logout</span>
            </button>
          </div>
        </div>
      </div>
    </nav>
  );
};

export default Navbar;