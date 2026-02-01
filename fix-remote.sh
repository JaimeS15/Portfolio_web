#!/bin/bash

# Fix Git Remote - Switch from SSH to HTTPS
echo "🔧 Fixing Git remote URL..."

# Remove SSH remote
echo "Removing SSH remote..."
git remote remove origin

# Add HTTPS remote
echo "Adding HTTPS remote..."
git remote add origin https://github.com/JaimeS15/Portfolio_web.git

echo ""
echo "✅ Remote updated to HTTPS!"
echo ""
echo "📋 Next step:"
echo "Run: git push -u origin main"
echo ""
echo "When prompted:"
echo "  Username: JaimeS15"
echo "  Password: Use a Personal Access Token (see FIX_GIT_REMOTE.md)"
echo ""
