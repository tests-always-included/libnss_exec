nss_exec
========

This is an extremely basic Name Service Switch module that will execute an arbitrary command so you can extend NSS easier with your amazing scripting abilities.


Caching for More Speed
----------------------

It is highly recommended that you run `nscd` or `unscd` (both are name service caching daemons) to avoid calling the script frequently.

When debugging, make sure to **not** run the caching daemon because they can mask problems by serving entries in the cache.


Compiling and Installation
--------------------------

There are virtually no external dependencies that are required.  The only thing that might be needed is `pthreads`.  Just `cd` into the `libnss_http` directory and run:

    # This builds the library
    make

    # Build the test tool as well
    make nss_test

Now you should write a script that handles the various calls your mechanism supports.  After you get the script written and installed as `/sbin/nss_exec` then you can install this library.

    # This installs it to the system
    sudo make install

Now update `/etc/nsswitch.conf` and add `exec` to the bits you support.  In this example we want the nss_exec to be called for passwd, group and shadow entries.

    passwd:         exec compat
    group:          exec compat
    shadow:         exec compat


Scripting and Debugging
-----------------------

The lookup script lives at `/sbin/nss_exec` and gets one or two parameters:

    # This is with only one parameter.
    # This call resets the passwd counter.
    /sbin/nss_exec setpwent

    # Here is a sample call with two parameters.
    # Gets the first passwd entry.
    /sbin/nss_exec getpwent 1

To help create and debug your script you should build `nss_test`

    # Build the test tool
    make nss_test

    # Run the test tool
    nss_test

The test tool will run `./nss_exec` so you can work on a copy of the script in the current directory and move it to `/sbin/nss_exec` once it is completed.

If things are not working with `sshd` or other services, double check your SELinux rules.  They can mess things up without giving you any diagnostic information in log files.


Credits
-------

Many thanks to [`libnss_http`](https://github.com/gmjosack/nss_http) for the beginnings of this project.


License
-------

This project is licensed under an [MIT License](LICENSE.md).
