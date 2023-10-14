# Contributing
Contributors are encouraged to be a [member](https://www.openhwgroup.org/membership/) of the OpenHW Group.  New members are always welcome.  Have a look at [README](https://github.com/openhwgroup/force-riscv/blob/master/README.md).

## The Mechanics
1. [Fork](https://help.github.com/articles/fork-a-repo/) the [force-riscv](https://github.com/openhwgroup/force-riscv) repository.<br>

2. Clone repository:  
   `git clone https://github.com/[your_github_username]/force-riscv`<br>
3. Create your feature branch:  
   `git checkout -b <my_new_branch>`<br>
     Please uniquify your branch name.  See the [Git Cheats](https://github.com/openhwgroup/core-v-verif/blob/master/GitCheats.md) for a useful nominclature.<br>
4. Test your changes.
5. Add your changes to git.  
   `git add *`
6. Commit your changes:  
   `git commit -sm 'Add some feature'`<br>...note that **-s** (signoff) is now optional.
7. Push feature branch:  
   `git push origin <my_new_branch>`<br>
8. Submit a [pull request](https://help.github.com/en/github/collaborating-with-issues-and-pull-requests/creating-a-pull-request-from-a-fork).  

## PyCodeStyle
[![Code style: black](https://img.shields.io/badge/code%20style-black-000000.svg)](https://github.com/psf/black)

We have recently joined many other fine software projects by adopting Black as our python code format standard.  In order to set your git ***pre-commit*** hook to check your changes automatically, please install pre-commit using either pip:  
    `pip install pre-commit`    
    
or, using the non-admin approach of:  
    `curl https://pre-commit.com/install-local.py | python3 -`

Then, from our project root folder, run:  
`pre-commit install`  

For complete details, visit:  
[pre-commit.com](http://www.pre-commit.com)


