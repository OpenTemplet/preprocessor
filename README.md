What is it?
-----------
This is a software developing toolkit for actor-oriented programming in general purpose languages like C++, C#, Java. Actors are basically concurrent processes that communicate by exchanging messages. With the toolkit you can compose programs in the form of actors network. This point of view gives advantages, if application areas are highly-parallel by its nature: multi-core programming, high performance computing, industrial control systems, Internet of Things, business process management, and many others.

Things that are important for the toolkit.
------------------------------------------
Actors and their communications are modeled in general purpose language (now in C++), no syntax extensions or API! So it takes minimal efforts from the developers to learn the toolkit. This feature also makes the toolkit compatible with all existing software infrastructure: languages, libraries, IDE's, your application code.

Components of the toolkit.
--------------------------
### OpenTemplet includes the following sub-projects:
* Templet markup language [preprocessor](https://github.com/OpenTemplet/preprocessor). The preprocessor forms actor-oriented structure (a skeleton) of your code. It is based on tiny and easy-to-learn language named Templet.

* Runtime libraries [cpp11runtime](https://github.com/OpenTemplet/cpp11runtime), [cpp98runtime](https://github.com/OpenTemplet/cpp98runtime) with samples. They implement actor model of execution in particular application area. Now we have runtime support for symmetric multiprocessing using C++11 and Windows/POSIX multithreading.

* A set of programming [skeletons](https://github.com/OpenTemplet/cppskeletons) for popular parallelism patterns such as bag-of-tasks (also known as farm or thread pool) and pipeline. They run on Windows API, MPI, POSIX.

* [Documentation](https://github.com/OpenTemplet/documentation) sub-project. We just in the beginning with documenting the toolkit. These are main papers about the technology in [English](http://arxiv.org/abs/1412.0981), and in [Russian](http://dx.doi.org/10.14498/vsgtu1334).

Templet developers.
-------------------
This project is supported by [Samara State Aerospace University](http://www.ssau.ru/english) (SSAU). The toolkit is now used in the web service [templet.ssau.ru](http://templet.ssau.ru/#eng) to build HPC apps for SSAU high performance cluster. This project is a part of learning and research activities of SSAU.

### Thanks for your interest in the project!
For more details feel free to contact:
Dr. Sergey Vostokin <sergey.vostokin@gmail.com>

