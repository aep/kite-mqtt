kite is "kind of like Qt"

In the 10 years that i worked with, on and for Qt, it hasn't changed much,
mainly because it doesn't have to. It's awesome for what it was built for.

I love Qt. Writing code with Qt is kindof like someone built the hard shit for me.
'Real C++' is terrible. It feels like a theroretical physics student
punishing me for being too intellectually inferior to understand string theory,
when all i wanted was eat cake all day and be happy.

Anyway, Qt has some issues that i was never able to address, even when they payd me to fix it:

- Qts build system is terrible. It's terible because Qt isnt well structured.
  And it isnt well structured becuase the build system is so terrible. (#include "moc_p.cpp" wtf)
  So it's kindof unfixable. Porting Qt to anything that isnt a desktop OS is just plain suffering.
  I did it like 4 times now (Nokia Emerging Poop, KorHal, Android, Heresy).
  Now at the 5th time trying to cram Qt down into a constrained mips device, i just gave up, because:

- Qt is big. And by big i mean obese. Tons of people have crammed tons of crap into it.
  For getting QString working, you need 164 other classes, one of which is a json parser.
  Because, yeah, qt plugins use json metadata, qstring has codings, which have plugins, bla bla.
  It's not really 'bad design' or anything, but most of this stuff is just irrelevant nowadays.
  Qt was designed when C++ string didnt even work, so they _had_ to make their own string class.

- All the paradigms are for guis. They were genious.... for guis. But no one writes Guis anymore.
  Signals & slots look cool for IO because they were awesome for GUIs.
  And then you drown in connect(this,A,this,Aslot) spaghetti, because all you needed was an observer.
  Qt's paradigma just dont fit into what i am doing with it (network components, cli apps)

So Kite is kindof like Qt, except obviously its only like the 1% of Qt that i need, that aren't
covered by C++. Don't be fooled, this is not boost. For example, I hate stdstreams, they're just
so mindboggling complicated. So Kite::IO just looks like QIODevice, i guess. On the other hand,
Kite doesnt have a codec library, because icu works fine.

