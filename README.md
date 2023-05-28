# S4_Projet_taskDaemon

## Compilation :
________________________________
### ⚠️ **Attention:** To use the following commands, make sure your terminal is in the project directory.
_______
> __1- Make sure to setup the library path with:__
>
> ```sh
> export LD_LIBRARY_PATH=$PWD
> ```
> 
 __2- To compil every programs of the project :__ 
> ```sh
> make
> ```
> 
 __3- To compil a specific program :__
> ```sh
> make PROG_NAME
> ```
>
>__4- To clean intermediate programs :__
>```sh
>make clean
>```
>
>__5- To clear every compiled programs :__
>```sh
>make mrProper
>```
>
>__6- To launch a program as a daemon:__
>```sh
>./launch_daemon $PWD/taskd
>```
>
>__7- To add a comand to be execute:__
>```sh
>./taskcli START PERIOD CMD [ARG]...
>```
>
>__8- To see the curent comands (plus the command that had a 0 as period):__
>```sh
>./taskcli START PERIOD CMD [ARG]...
>```
>
>__7- To delete a comandéz :__
>```sh
>./tascli -d NUM_CMD
>```

______
## Summary

#### On the final product:

In this project, all the exercises have been completed and tested, including the bonus exercise. It should be noted that commands with a period of 0 are not removed from the file `/tmp/task.txt` because otherwise it would be impossible to see the results of these commands. Therefore, when using `./tascli` to view the executing commands, commands with a period of 0 also appear. Of course, they cannot be deleted with `./tascli -d NUM_CMD` even if they are present in `/tmp/task.txt`.

#### On teamwork:

During this project, we were able to organize ourselves thanks to the GitHub version control. The workload was very unbalanced as one of us was more comfortable with C. Considering the different projects we had to deal with, the workload was rather distributed across all three projects.


