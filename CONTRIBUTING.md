CI Tests
========
We use various continous integration platforms and a testbench to
ensure good code quality.

All pull request MUST pass all CI tests in order to be considered
for merging.

If bugs are fixed or new functionality is provided, it is highly
suggested to add a related test to the testbench. If not done,
the merge may be rejected. This is not a hard rule as for some
situations it may be very hard and even impossible to craft an
automatic tests. This should not be used as an excuse for
lazyness.

Code Style
==========
Unfortunaly, code style has not been officially described in the
past. As such, some parts of the code do not yet fully conform to
what we really want.

Also, the rsyslog team has not yet fully agreed on a formal description
of the coding style. This is currently under discussion.

Here are the minimal style guidelines to ensure code will pass
automatted code style checks during CI runs:

* indentions are done via TAB, not spaces
* no trailing whitespace is permitted at the end of line
