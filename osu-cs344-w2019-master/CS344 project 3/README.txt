Author:: Hongyiel Suh
Class :: CS344
FileName:: smallish.c

----------------------------------------------------------------------------
Overall structure of shell
----------------------------------------------------------------------------

* Status 	:: it will give signal that using exit value
* CatchSignal 	:: literally compare signal and wait for child processor
* Ctrl +handler :: Handling Ctrl command (background job and foreground job)
  - Using signal struct on the main function to control "ctrl +" command

* Tokenizing 	:: get tokens individually
  - get string array individually (and in the command_hander function, it will change
    array that with out special letter)

* Command ctr 	:: This is actual command Handler
  - Control "file I/O"
  - Control specific command (ex. Cd, exit, status)
  - Control child fork here

* main
  - get $$ here
  - exit when while loop is done

----------------------------------------------------------------------------

