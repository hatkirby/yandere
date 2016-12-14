# yandere
Simple text substitution Twitter bot in the vein of [chemist](https://github.com/hatkirby/chemist), but without the dependency on [verbly](https://github.com/hatkirby/verbly). Tweets every hour, generating text based off of a data file constructed with a simple syntax.

It uses my Twitter library [libtwitter++](https://github.com/hatkirby/libtwittercpp) to post to Twitter, and [YAMLcpp](https://github.com/jbeder/yaml-cpp) to read a configuration file.

Examples of `yandere` bots:

- [@bbbbbbbbbbbaka](https://twitter.com/bbbbbbbbbbbaka) (d-su), which posts inside jokes that no one understands

## Syntax

The data file is split up into "groups", which are lists of options. The start of a group is denoted by its name in all caps, followed immediately by a line-seperated list of options. Each group must be separated by at least one empty line. Example:
```
MAIN
Here's an option.
This is another one.
Guess what!

SECOND_GROUP
This group only has two options.
Here's the other one.
```

A group can be mentioned within an option using the syntax `{GROUP}`. This is called transclusion. When the bot would generate text including a transclusion, it replaces it with a uniformly-at-random picked option from that group. Example:
```
MAIN
{SECOND_GROUP}
{SECOND_GROUP} {SECOND_GROUP}

SECOND_GROUP
foo
bar
```

The datafile above can generate the following outputs:
```
foo
bar
foo foo
foo bar
bar foo
bar bar
```

This is the primary way in which the bot generates tweets; it begins with the text `{MAIN}` and transcludes recursively until there are no remaining transclusions. Example:
```
MAIN
foo
foo {MAIN}
```

The datafile above can generate `foo`, `foo foo`, `foo foo foo`, and so on.

Transclusions support a few basic modifiers, which are denoted with the syntax `{GROUP:modifier}`. Currently, the only modifier is `indefinite`, which places either "an" in front of the transcluded text if it is all caps or begins with a vowel, and places "a" otherwise.

Transclusions can also be stored in variables so that they can be reused. This is denoted with the syntax `{VAR=GROUP}`. When the bot encounters this transclusion, it generates a replacement as it would for `{GROUP}`, and then essentially creates an imaginary new group called `VAR` into which it places the generated replacement. That group can later be transcluded from using `{VAR}` as per usual. Example:
```
MAIN
{VAR=SECOND_GROUP} {VAR}

SECOND_GROUP
foo
```

The datafile above would generate the text `foo foo`.

Finally, you can modify the capitalization of a transclusion by changing the casing of the group's name. Referencing a group in all caps transcludes text as normal, referencing it in all lowercase lowercases the generated text, and referencing it with the first letter uppercase and the second letter lowercase will title case the generated text (specifically, it will capitalize the first letter of each generated word). Example:
```
MAIN
{SECOND_GROUP}
{second_group}
{Second_group}

SECOND_GROUP
FOO BAR
```

The datafile above can generate either `FOO BAR`, `foo bar`, or `Foo Bar`.

There is one "special" group name: `{\n}` is always replaced by a newline.

## Configuration

When you run `yandere`, you pass it the path of a configuration file that tells the bot where to look for its datafile, which it calls "forms". As explained above, this datafile should at least have a group named `MAIN`. The bot also reads the details for authenticating with Twitter from this configuration file. This file should have the following format, with the appropriate tokens inserted between the appropriate pairs of double quotes:
```yaml
---
  consumer_key: ""
  consumer_secret: ""
  access_key: ""
  access_secret: ""
  forms: "data.txt"
```
