# **![Koder icon](./artwork/Koder_icon_64.png)** Koder

Koder is a code editor built using Scintilla editing component.

![Screenshot](./artwork/Koder.png)

## Dependencies

* Scintilla >=4.1.3
* yaml-cpp
* Scintilla [lexers](https://github.com/KapiX/scintilla-haiku-lexers) for Haiku specific file types
* GTest (to run the tests)

## Building

Koder uses makefile_engine. Invoke make in root directory.

## Installation

Copy all files from **data** directory to **Koder** directory in any non-packaged/data folder.

## Running tests

```
make check
```

## Contributing

This project follows Haiku coding guidelines (more or less).

When implementing new features, bear in mind that Koder is an editor and not an IDE. A good benchmark for new features is whether it works on a file or a group of files. In Koder 1 window = 1 file.

That being said, creating interfaces to talk with an IDE is fine. This approach comes from Haiku philosophy of having small programs doing one thing and talking to each other. Like in Unix, but with GUI.

## [Changelog](CHANGELOG.md)

## Release checklist

- [ ] Update translations and credits in About window
- [ ] Update README, changelog and screenshot
- [ ] Create release branch
- [ ] In release branch:
	- [ ] Change release date in changelog
	- [ ] Disable debug and symbols in makefile
	- [ ] Change version to final in rdef
	- [ ] Commit "Release {version}"
	- [ ] Create tag and a release
- [ ] In master branch:
	- [ ] Change release date in changelog and add a new version
	- [ ] Bump version in rdef
	- [ ] Commit "Bump to {next-version}-dev"
