# Transition Fixer

This command line tool can be used to fix an issue that occurs in Windows 7 and higher where changing your PC's wallpaper doesn't cause the fade transition to play.

This is due to Active Desktop not being enabled. Under the hood, to fix this issue, you must call `SendMessageTimeout`, sending a message of `0x52C` to the `Progman` window. For more information, see this [StackOverflow post](https://stackoverflow.com/questions/14773287/iactivedesktop-wallpaper-fade-effect-not-working-after-restart).

## License

This project is licensed under the [MIT License](https://opensource.org/licenses/MIT). For more information, refer to the [`LICENSE.md`](LICENSE.md) that is in the repository.