#!/bin/bash

# Git Setup Script for cPanel Deployment
# Run this script to initialize Git and prepare for deployment

echo "🚀 Setting up Git for cPanel deployment..."

# Initialize Git repository
echo "📦 Initializing Git repository..."
git init

# Add all files
echo "➕ Adding files to Git..."
git add .

# Create initial commit
echo "💾 Creating initial commit..."
git commit -m "Initial commit: Portfolio website with RunAR project"

# Set main branch
echo "🌿 Setting main branch..."
git branch -M main

echo ""
echo "✅ Git repository initialized successfully!"
echo ""
echo "📋 Next steps:"
echo "1. Create a repository on GitHub (github.com → New repository)"
echo "2. Copy the repository URL"
echo "3. Run: git remote add origin YOUR_REPOSITORY_URL"
echo "4. Run: git push -u origin main"
echo "5. Set up Git Version Control in cPanel (see GIT_DEPLOYMENT_GUIDE.md)"
echo ""
