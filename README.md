Citrine
=======

Welcome to the Citrine Programming Language Project.

Citrine is a __general-purpose, localised scripting language.__ It aims to foster readable
and maintainable code, while remaining simple and easy to learn by focusing on syntactical
and conceptual minimalism:

```
☞ greeting ≔ ‘Hello country!’.

greeting country: ‘Great Britain’.

✎ write: greeting.
```

> Hello Great Britain!

Citrine is wholly [free](https://www.gnu.org/philosophy/free-sw.html) and
[open-source](https://opensource.org/osd) software, released under the 2-clause BSD
License (see the file `LICENSE`).

For more information (including more examples and an on-line demo), please see the
[official website](https://citrine-lang.org/).


Installation
------------

Citrine is cross-platform and should run on a variety of operating systems, but the
installation procedure for some is currently lacking in support. Help to improve the
situation is very welcome.

Please make sure you are using the latest official release of Citrine, in order to have
the newest features and bug fixes.

### Using a precompiled binary

The easiest way to get started is by using a precompiled binary, currently available on
the [official website](https://citrine-lang.org/download.ctr) for the following systems:

- GNU/Linux
- Microsoft Windows
- OpenBSD

### Compiling the source code

Source code for the latest official release may likewise be acquired from the [official
website](https://citrine-lang.org/download.ctr). If you choose to compile it yourself,
there’s an installation script, `mk.sh`, included to automate the process. You may run it
as follows:

```bash
./mk.sh
```

This should generate a separate binary for every supported language in the `bin`
directory, in a subdirectory matching your system. Now you can install Citrine with all of
the compiled binaries using `make`:

```bash
make install
```

You can adjust the path the files get installed to by setting the variable `prefix`, if
the default value doesn’t work well for your setup:

```bash
make prefix='/usr/local' install
```

#### Compiling only some languages

Compiling binaries for all the languages can take a while. If you know for sure that you
won’t be needing all of those, you can choose to only compile binaries for a subset of the
languages instead, by passing their codes as arguments to `mk.sh`:

```bash
./mk.sh en nl  #Compile English and Dutch only.
```

Without arguments, `mk.sh` compiles all the languages listed in the `i18nsel` directory.
By default, this is only a symbolic link to the actual directory `i18n`, where all of the
available languages are really stored.

#### Manual compilation

In case `mk.sh` doesn’t work for you, here’s an explanation of how to compile Citrine
manually, but still using `make`:

First, a binary for every language is compiled separately. Every time you run `make` on
the appropriate makefile, the environment variable `ISO` of your shell should contain the
ISO code of the language you want to compile. Likewise, the variable `OS` should contain
an identifier of your operating system. For example:

```bash
ISO='hi' OS='Haiku' make -f makefile all
```

Given the above, `make` would fetch Hindi vocabularies from the language’s files in
`i18n/hi`, compile them into a binary named `ctr`, and copy the binary to the Haiku
system’s directory at `bin/Haiku`, with the language’s code attached to the filename.

Now’s a good opportunity to use the binary to compose a dictionary of translations between
two languages. Just point Citrine to each language’s `dictionary.h` file to make a one-way
translation dictionary:

```bash
./ctr -g i18n/nl/dictionary.h i18n/hi/dictionary.h > dict/nlhi.dict
./ctr -g i18n/hi/dictionary.h i18n/nl/dictionary.h > dict/hinl.dict
```

These two commands would produce two dictionaries in the `dict` directory: one for
translating from Dutch to Hindi, and another for translating from Hindi to Dutch.

Having done all this, you can clean up the files produced during compilation and move on
to the next language:

```bash
make -f makefile clean
```
Once you’ve compiled all the languages you want, you will find the binaries in `bin`. You
can start using them right away, but you might want to install them under your system’s
standard binary path first. You also probably want to install the included font that gives
Citrine’s symbols a nicer look in your text editor:

```bash
mv bin/Haiku/ctr* ~/.local/bin
mv fonts/Citrine.ttf ~/.local/fonts
```

And with this, you should be done!


Contributing
------------

If you’d like to contribute to the project, you can get in touch using e-mail or GitHub.
Forms of contribution include, but are not limited to:

- Ideas for improvement.
- [Code improvements](https://citrine-lang.org/changelog.ctr#roadmap).
- [Translations](https://citrine-lang.org/add.ctr).
- [Plugins](https://citrine-lang.org/changelog.ctr#plugins).

Citrine and its plugins are written in the __C__ programming language. The Citrine Project
is apolitical.


<a target="_blank" href="https://travis-ci.org/gabordemooij/citrine.svg?branch=master">
<img alt="test status" src="https://travis-ci.org/gabordemooij/citrine.svg?branch=master" />
</a>
