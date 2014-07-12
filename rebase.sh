git stash
git checkout master
git pull --rebase origin
git branch | grep / | while read b; do git rebase master $b; done
git branch -D merged.old
git branch -M merged merged.old
git checkout -b merged master
git branch | grep / | while read b; do git merge $b; done
git stash pop
