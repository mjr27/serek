serek
=====

Implementation of PHP serialization for Python


A Python module which implements PHP serialization. It is written in C, thus it should be faster than other, pure-Python implementations.Why would you ever want to use PHP serialization in Python? (Intead of, say, JSON?) Well, if you have a system consisting of both Python and PHP parts, you may want to talk to the PHP parts in their own language. But, well, most likely you will use it when you have some old legacy PHP code, and want to build newer parts of the system in Python.There's no release available yet. However, current code seems to be stable and was heavily used in production, so you might want to grab the source straight from the Mercurial repository if you're desperate.After you import the module, you can use either PHP-like named functions: serialize and unserialize, or its Python-like equivalents: dumps and loads:


    >>> import serek
    >>> x = serek.dumps({1: "foo", "bar": [2, 4, 8]})
    >>> x
    'a:2:{i:1;s:3:"foo";s:3:"bar";a:3:{i:0;i:2;i:1;i:4;i:2;i:8;}}'
    >>> x == serek.serialize({1: "foo", "bar": [2, 4, 8]})
    True
    >>> serek.loads(x)
    {1: 'foo', 'bar': {0: 2, 1: 4, 2: 8}}

Object serialization is not supported (yet). The legacy PHP system that the author had to deal with doesn't use such things (fortunately!). BTW, an idea: Maybe substituting full-featured objects with nested dicts is doable and worth considering?

This respository is a clone of [http://code.google.com/p/serek/](http://code.google.com/p/serek/) by Marcin Wrochniak.

