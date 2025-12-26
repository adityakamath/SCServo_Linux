#!/bin/bash
# Script to sync with_ai_docs branch with main while preserving docs/ folder
# Usage: ./sync_docs_branch.sh

set -e

echo "Syncing with_ai_docs branch with main..."

# Stash any uncommitted changes
git stash

# Switch to with_ai_docs branch
git checkout with_ai_docs

# Merge main into with_ai_docs
git merge main -m "Merge main into with_ai_docs: sync latest changes"

# Restore docs folder from previous commit (before merge)
git checkout HEAD~1 -- docs/

# Commit the restored docs
git commit -am "Restore docs folder after merge with main"

# Push to remote
git push origin with_ai_docs

echo "✓ Sync complete! with_ai_docs branch now contains all changes from main + docs/"
