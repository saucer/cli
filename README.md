<hr>

<div align="center"> 
    <img src="https://raw.githubusercontent.com/saucer/saucer/master/assets/logo.png" height=312/>
</div>

<p align="center"> 
    Build cross-platform desktop apps with C++ & Web Technologies 
</p>

---

<div align="center"> 
    
This project hosts the source code of the command line utility.  
For more information see [our documentation](https://saucer.github.io/).

</div> 


# Installation

<div align="center"> 

<img src="https://www.vectorlogo.zone/logos/npmjs/npmjs-ar21.svg" height=20/><br/>
[Available from npm](https://www.npmjs.com/package/saucer-app) <br/>
<code>sudo npm i -g saucer-app</code>
<code>sudo yarn global add saucer-app</code>

</div>

# Recommended Usage

It is recommended to use `saucer embed` inside of your build script.

For example in your `package.json`:

```json
{
  // ...
  "scripts": {
    "build": "<deploy normally> && saucer embed dist"
  }
}
```

# Synopsis

```
Usage: saucer [options] [command]

Options:
  -h, --help                    display help for command

Commands:
  embed <source> [destination]  Generate embedding headers for given files
  help [command]                display help for command
```