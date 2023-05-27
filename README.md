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
 __2- To compil every programs of the project :__ 
> ```sh
> make
> ```
>
 __3- To compil a specific program :__
> ```sh
> make < program-name >
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
