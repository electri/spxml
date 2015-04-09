spxml is a simple and plain xml parser written in C++. spxml supports [pull-model](http://www.xmlpull.org/) and DOM-model xml parsing.

spxml is a stream-oriented parser. As the user passes it chunks of the input XML document, spxml identifies elements, character data or other entities and return the appropriate event. The size of the chunks is controlled by the user; chunks can range from one byte to the whole XML document.

spxml pull parser does not need to be told when the document is ended. On the contrary, spxml pull parser will tell us when it has finished parsing a root ( or other ) element. spxml pull parser can therefore safely read from pipes, can process sequences of XML documents without extra delimiters, and can handle selected parts of a document.

spxml DOM parser parses an XML document, and builds a DOM tree that can be read, modified, and saved.

```
Changelog:

Changes in version 0.5 (10.02.2007)
-------------------------------------
* SP_XmlHandle was added, it wraps a node pointer with null checks

Changes in version 0.4 (07.22.2007)
-------------------------------------
* Add utf8 and processing instruction support
* Test with James Clark's valid/sa testcases, 81 pass, 38 fail (almost about xml entity)

Changes in version 0.3 (03.02.2007)
-------------------------------------
* Add canonical xml (http://www.jclark.com/xml/canonxml.html) test tool
* Test with James Clark's valid/sa testcases (ftp://ftp.jclark.com/pub/xml/xmltest.zip), 69 pass, 50 fail (almost about xml entity)
* Test under windows, add MS Visual C++ project files

Changes in version 0.2 (02.18.2007)
-------------------------------------
* bugfix -- spxml cannot process empty tag look like:
  <tagname
     attr='val'
  />
* Performance improvements: replace ArrayList by CycleQueue in PullParser
```