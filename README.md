# **![Koder icon](./artwork/Koder_icon_64.png)** Koder

Koder is a code editor built using Scintilla editing component.

![Screenshot](./artwork/Koder.png)

## Dependencies

* Scintilla
* yaml-cpp

## Building

Koder uses makefile_engine. Invoke make in root directory.

## Installation

Copy all files from **data** directory to **Koder** directory in any non-packaged/data folder.

## History

0.4.1 - *23-11-2018*

* Fix crash in incremental search.
* Fix saving of Spaces per tab setting.

0.4.0 - *07-10-2018*

* Find/Replace window can be closed with Esc now.
* Fix for saving preferences.
* Save Find window settings.
* Limited .editorconfig support.
* Stack all windows besides those from Tracker.
* Add status bar.
* Add dark theme by Mikael Konradsson.
* Add support for HTML, CSS, JavaScript, PHP and OCaml.
* Change styling system to make creating new themes easier.
* Add quick search shortcuts.
* Implement incremental search.
* Fix horizontal scroll bar range.
* Add setting for line highlighting mode.
* Add option to always open files in new windows.
* Add option to change font.

0.3.0 - *20-03-2018*

* New icon.
* Add right-click menu.
* Disable menu items according to editor state.
* Fix multiple language entries in Language menu.
* Show alert if styles not found.
* Change shortcut for Go to line.
* Add View->Wrap lines.
* Add File->Reload.
* Add tooltips to toolbar buttons.
* Add option to attach windows to existing ones.
* Cascade windows if necessary.
* Open files in the same window only if it is new window and has not been modified.
* Add option to highlight trailing whitespace.
* Add option to trim trailing whitespace.
* Add option to trim trailing whitespace automatically on save.
* Use multiline controls in Find window.
* Add support for regex in Find/Replace.
* Fix keyboard navigation in Find/Replace.
* Rearrange Find/Replace window interface.
* Add "Open corresponding file" option.
* Fix opening files from command line.

0.2.0 - *15-04-2017*

* Add icon.
* Add translations.
* Adjust whitespace size.
* Fix crash when changing git branches with a file open.
* Fix horizontal scrollbar on long lines.
* Recolor the document on language change.
* Handle drag and drop message.
* Add jumping to specific position in file from command line. Handle parameters from /bin/open.
* Add comment line/block feature.
* Add indent guides highlighting.
* Add editor style menu to preferences.
* Add toolbar.
* Put selection in Find window automatically.
* Add AWK, Perl, Ruby and Rust support.

0.1.0 - *01-01-2017*

* Initial release.