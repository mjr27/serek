#!/usr/bin/python
import unittest

import serek

class TestSerekSerialization(unittest.TestCase):

    def tearDown(self):
        reload(serek)

    def _make_pair(self, dst, src):
        self.assertEquals(dst, serek.serialize(src))
        self.assertEquals(src, serek.deserialize(dst))
    
    def test_string(self):
        self._make_pair('s:3:"abc";', 'abc')
        self._make_pair('s:0:"";', '')
        self._make_pair('s:1:""";', '"')
        self._make_pair('s:6:"ab"cd"";', 'ab"cd"')
        self.assertRaises(ValueError, lambda: serek.deserialize('s:3:"abc";x'))
        self.assertRaises(ValueError, lambda: serek.deserialize('s:5:"abc";'))
        self.assertRaises(ValueError, lambda: serek.deserialize('s:3:"abcde";'))
        self.assertRaises(ValueError, lambda: serek.deserialize('s3:"abc";'))
        self.assertRaises(ValueError, lambda: serek.deserialize('s:3"abc";'))
        self.assertRaises(ValueError, lambda: serek.deserialize('s:3:"abc"'))

    def test_int(self):
        self._make_pair('i:123;', 123)
        self._make_pair('i:-123;', -123)
        self._make_pair('i:0;', 0)
        self.assertEquals(type(1), type(serek.deserialize('i:4312;')))
        self.assertTrue(isinstance(serek.deserialize('i:4312;'), int))
        self.assertRaises(ValueError, lambda: serek.deserialize('i:123;x')) # Garbage data at the end
        self.assertRaises(ValueError, lambda: serek.deserialize('ii:123;'))
        self.assertRaises(ValueError, lambda: serek.deserialize('i'))
        self.assertRaises(ValueError, lambda: serek.deserialize('i1'))
        self.assertRaises(ValueError, lambda: serek.deserialize('i:123'))
        self.assertRaises(ValueError, lambda: serek.deserialize('i:123{;'))
        self.assertRaises(ValueError, lambda: serek.deserialize('i123{;'))

    def test_long(self):
        self._make_pair('i:12399999999999999999999999;', 12399999999999999999999999)
        self._make_pair('i:-12399999999999999999999999;', -12399999999999999999999999)
        self.assertEquals(type(199999999999999999999999), type(serek.deserialize('i:431299999999999999999999999;')))
        self.assertTrue(serek.deserialize('i:431299999999999999999999999;'), long)
        self.assertRaises(ValueError, lambda: serek.deserialize('i:12399999999999999999999999;x')) # Garbage data at the end
        self.assertRaises(ValueError, lambda: serek.deserialize('ii:12399999999999999999999999;'))
        self.assertRaises(ValueError, lambda: serek.deserialize('i:12399999999999999999999999'))
        self.assertRaises(ValueError, lambda: serek.deserialize('i:12399999999999999999999999{;'))
        self.assertRaises(ValueError, lambda: serek.deserialize('i12399999999999999999999999{;'))

    def test_none(self):
        self._make_pair('N;', None)
        self.assertRaises(ValueError, lambda: serek.deserialize('N;x'))
        self.assertRaises(ValueError, lambda: serek.deserialize('N'))
        self.assertRaises(ValueError, lambda: serek.deserialize('Na;'))
        self.assertRaises(ValueError, lambda: serek.deserialize('NN'))

    def test_bool(self):
        self._make_pair('b:1;', True)
        self._make_pair('b:0;', False)
        self.assertRaises(ValueError, lambda: serek.deserialize('b:1;x'))
        self.assertRaises(ValueError, lambda: serek.deserialize('b:0;x'))
        self.assertRaises(ValueError, lambda: serek.deserialize('b1;'))
        self.assertRaises(ValueError, lambda: serek.deserialize('b:1'))
        self.assertRaises(ValueError, lambda: serek.deserialize('b:10'))
        self.assertRaises(ValueError, lambda: serek.deserialize('b:11'))
        self.assertRaises(ValueError, lambda: serek.deserialize('b11'))
        self.assertRaises(ValueError, lambda: serek.deserialize('b'))
        self.assertRaises(ValueError, lambda: serek.deserialize('b:2;'))
        self.assertRaises(ValueError, lambda: serek.deserialize('b:-1;'))
        self.assertRaises(ValueError, lambda: serek.deserialize('b:a;'))
        self.assertRaises(ValueError, lambda: serek.deserialize('b:2'))
        #self.assertEquals(True, serek.deserialize('b:2;'))
        #self.assertNotEquals(True, serek.deserialize('b:-1;'))

    def test_general_parse(self):
        self.assertRaises(ValueError, lambda: serek.deserialize('#'))
        self.assertRaises(ValueError, lambda: serek.deserialize('#i:123;'))
        self.assertEquals(None, serek.deserialize(''))

    def test_tuple(self):
        self.assertEquals('a:3:{i:0;i:1;i:1;i:2;i:2;i:3;}', serek.serialize((1, 2, 3)))
        self.assertEquals('a:4:{i:0;a:3:{i:0;i:1;i:1;i:2;i:2;i:3;}i:1;i:4;i:2;a:2:{i:0;i:5;i:1;i:6;}i:3;i:7;}',
                          serek.serialize(((1,2,3),4,(5,6),7)))
        self.assertEquals('a:5:{i:0;b:1;i:1;s:4:"test";i:2;i:123;i:3;i:-123;i:4;a:2:{i:0;s:1:"a";i:1;b:0;}}',
                          serek.serialize((True, 'test', 123, -123, ('a', False))))

    def test_list(self):
        self.assertEquals('a:3:{i:0;i:1;i:1;i:2;i:2;i:3;}', serek.serialize([1, 2, 3]))
        self.assertEquals('a:4:{i:0;a:3:{i:0;i:1;i:1;i:2;i:2;i:3;}i:1;i:4;i:2;a:2:{i:0;i:5;i:1;i:6;}i:3;i:7;}',
                          serek.serialize([[1,2,3],4,[5,6],7]))
        self.assertEquals('a:5:{i:0;b:1;i:1;s:4:"test";i:2;i:123;i:3;i:-123;i:4;a:2:{i:0;s:1:"a";i:1;b:0;}}',
                          serek.serialize([True, 'test', 123, -123, ['a', False]]))

    def test_map(self):
        self._make_pair('a:1:{i:1;s:1:"a";}', {1:'a'})
        self._make_pair('a:2:{s:1:"a";b:0;i:1;a:2:{b:0;b:1;i:1;i:2;}}', {'a':False,1:'abc',True:{1:2,False:True}})

    def test_deserialize_array(self):
        self.assertRaises(ValueError, lambda: serek.deserialize('a:2:{i:0;}'))
        self.assertRaises(ValueError, lambda: serek.deserialize('a:1:{i:0;i:2}'))
        self.assertRaises(ValueError, lambda: serek.deserialize('a:1:{i:0;i:2;};'))
        self.assertRaises(ValueError, lambda: serek.deserialize('a:2:{i:0;i:1;}'))
        self.assertRaises(ValueError, lambda: serek.deserialize('a:2:{i:0;i:1;i:1;i:2;i:3;i:4;}'))

    def test_mixed(self):
        self.assertEquals(serek.serialize({0:1,1:1,2:'a'}), serek.serialize((1,1,'a')))


if __name__ == '__main__':
    unittest.main()

