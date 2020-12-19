---
id: tree_hygiene
title: Tree Hygiene
sidebar_label: Tree Hygiene
---

## tl;dr

- Aside from documentation changes, all changes require tests. If you believe you can be exempt, reach out to @Calamity210 for approval
- All tests must pass. Take caution when changing existing tests.
- Expect PRs to be reviewed within a week, if you haven't gotten a review, reach out to any of the team members.

## Overview

The general process for submitting a PR to Senegal is:

1. Fork the repository on GitHub (see the [Contribution guide](https://github.com/SenegalLang/Senegal/blob/master/CONTRIBUTING.md)).

2. If there is not already an issue covering the work you are interested in doing, feel free to open one describing what you intend to work on as long as it is non-trivial.

3. Create a branch off master on your GitHub fork of the repository, and implement
   your changes along with test.

4. Submit this branch as a PR.

5. Get your code reviewed (see below).

6. Make sure your PR passes all the tests. Consider running some of the tests locally (cd test && pub run test).

7. Once everything is green and you have an LGTM, a team member will merge your PR.

## Using git

 * `git fetch upstream`
 * `git checkout upstream/master -b branch_name`
 * Make changes.
 * `git commit -a -m "<your commit message>"`
 * `git push origin branch_name`

GitHub provides you with a link for submitting the pull request in the message output by `git push`.

Please make sure all your PRs have detailed commit messages.

## Getting a code review

Every PR must be reviewed before being merged, getting a review means that a team member (someone with commit access) has either written sent a
"LGTM" ("Looks Good To Me") or approved your PR.

If nobody reviews your PR within two weeks, you can ask for a review by mentioning a team member or asking in our [Discord Server](https://discord.gg/AacmA3W)
