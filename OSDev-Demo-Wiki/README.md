#OSDev-Demo

##Demonstrator for OSDev.org

Full description: http://Wiki.OSDev.org/JohnBurger:Demo

###INSTRUCTIONS

####Just to try:
1. Download `Demo.iso` (46 kiB)
2. Burn `Demo.iso` to a CD -or- use file as a CD in
   [VMware](https://my.vmware.com/web/vmware/free#desktop_end_user_computing/vmware_player/6_0),
   [VirtualBox](https://www.virtualbox.org/wiki/Downloads) or
   [Virtual PC](http://www.microsoft.com/en-au/download/details.aspx?id=3702)

    (Use "Other" OS with only 4 MB RAM - no hard drive is required)
3. Boot `Demo.iso`, sit back, and admire!
4. Press &lt;Break&gt; on the keyboard, and use &lt;Up&gt;, &lt;Down&gt;, &lt;PgUp&gt;, &lt;PgDn&gt;, &lt;Left&gt;, &lt;Right&gt;, &lt;Del&gt; and &lt;Esc&gt;.
5. Press &lt;Break&gt; again. And again. And again. And...

####To Experiment:
1. Use GitHub's "Download ZIP" link to get all these files at once -or- Clone the repository for a full Git environment.
2. Install the [NASM](http://www.nasm.us/) assembler.
3. Under Windows, I suggest using [PSPad](http://www.pspad.com/), but only because a `.ppr` (PSPad Project) file is included.
4. Read the [Demonstrator Overview](http://Wiki.OSDev.org/JohnBurger:Demo/Overview), especially  the [Experiments](http://Wiki.OSDev.org/JohnBurger:Demo/Overview#Experiments) section.
5. The NASM command line is
```sh
nasm -o Demo.iso Demo.asm
```
