# Cracklock

![image](https://user-images.githubusercontent.com/3913258/132970728-e71552c1-df04-4f7d-8c5d-7d7086b2d7d3.png)
This project hosts the sources of [Cracklock](https://william.famille-blum.org/software/cracklock/).
I am sharing them here to celebrate the 24 years anniversary of the initial release of Craklock on November 1st, 1997.

Be warned that some of the source code may be of very low quality by today's standards, and probably even 24 years ago!
I was a young student back then and got into this project in my spare time as a way to learn C++, Win32 programming and API hooking.
To give you an idea, I used tabs instead of spaces for code indentation, so I obviously had no clue what I was doing.
And don't even get me started on the programming language. C++ was all the rage back then, and Rust did not exist yet. Oh, and to spice things up, I wrote all the code comments in French. I used some obscure European character encoding so you need to adjust your editor  encoding to properly display the accentuated characters.

May I also mentioned that you should not use Cracklock to break shareware protections.
This is illegal (in most countries) and I certainly don't want you to get in legal troubles.
If you are a poor student who cannot afford the license for a unique piece of software with no free alternative and that's critical to your personal growth and development then... you have my sympathy. But from my limited understanding of the law, I don't think this is a valid exceptions to your situation.
An attorney would more reliably advise on the matter, unfortunately they tend to be somewhat more expensive than software licenses.
Anyway, you probably could not care less about advice from an old fart, so it's really up to you at the end of the day.

I suggest exploring legitimate applications of the techniques underpinning Cracklock. For instance, software testing under deterministic system time, running multiple applications concurrently with different timezones, learning how to implement Win32 API hooking, running apps securely via sandboxing...

# Building

The original version of Cracklock was built with MSVC 4.0 and then got regularly migrated to subsequent versions of the MS C++ compiler.
I have not tried building it on the latest version of MSVC, but if it does not build successfully it should be straightforward to fix any compilation error you may encounter.

The original version of Cracklock supported both 16-bit and 32-bit and targeted both Windows 95 and Windows NT.
I believe this latest version of the code only has support for 32-bit but I don't quite remember. I never got to implement support for 64-bit.

If someone finds time to make the project build using GitHub actions please send a PR.

# Contributions

This is just a dump of the code base from a zip archive that I recovered from an old hard-drive.
Many people asked me for access to sources over the years but I never found time to properly prepare for a code release so I just decided to dump the code as it is. I am not certain it is useful to anyone out there anymore. I am not committing to actively maintaining this project.
If someone has any interest in this and feels like contributing, I would recommend forking the project. I may still accept PRs but probably not substantial ones.

# License

The code is released under [MIT license](/LICENSE).
