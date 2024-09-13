# Abstract <br>
The project proposed here is the development of an NDI based system that will be used for
esports broadcasting (most likely local tournaments), managing multiple incoming video feeds
from players/streaming devices, game streaming data and commentators. The product is intended
to improve the quality of esports events at any level, although we expect it will help the
smaller/local organizations around the world that may not have access to the infrastructure of a
large company the most. This will be done through NDI technology allowing low latency
without sacrificing quality, and we will develop the ability for our application to real time swap
between feeds in order to manage the output stream, and the output stream can then be
transmitted in the most effective manner.
<br> <br>
# Introduction <br>
The formation of our problem came from the simple fact that esports broadcasting requires a
great deal of synchronization across multiple devices and a minimization of input lag to make
these streams come together ‘at the same time’. Player webcams, commentator views/webcams,
and gaming video sources are the most common streams needed to be integrated into one output,
and this usually takes a lot of complex and expensive hardware that takes a lot of time to
manage. Reducing this barrier to entry will help smaller organizations and clubs be able to
stream and put on their events. This NDI based program will tone down the complicated aspects
of the broadcasting setup process for the layman user, simplifying the process using network
driven architecture. NDI uses multiple incoming video sources to be shared over a network,
lessening the dependency on hardware and software made specifically for broadcasting.
Our proposed solution will allow esports broadcasters to manage multiple feeds live, as well as
swap between the streams manually or automatically due to game events. Additionally, overlays
that can be customized for whatever the individual commentator wishes to display will allow
them to set it up without a post effects team, minimizing the crew needed to set up a broadcast.
The NDI technology we will be using in our development will allow all of this with minimal
latency and maximum quality available within the network’s restrictions. The impact of this
project could be immense in regards to small time groups/organizations being able to finally get
over the hurdle of not having enough computer equipment to put on a robust, quality stream that
only lacks complete dependency when compared to a standard broadcast setup. For larger events,
the effects will be more noticeable as there would be less hardware and setup to deal with, with
potential for great reduction in operating costs.
<br> <br>
# Literature Survey <br>
Esports broadcasting is a costly affair and while grassroots movements remain the core of
esports, the broadcast components are still catching up.This literature review examines the
existing research and industry practices surrounding esports broadcasting, with a particular focus
on the development of an accessible production application. Why is there a need for the
development of a user-friendly esports production application utilizing NDI technology?
The literature in broadcast technology is extremely thorough in some places, like trends
and their effects over the past 50 years, and lacking in others like discussing the technology
behind broadcasts that have been employed to deliver content to audiences, ranging from
traditional broadcasting equipment to modern software-based solutions. Many of the articles and
research pieces that can be read about include product descriptions and user reviews (1,2,3).
NewTek’s whitepaper on NDI describes what the technology is capable of and why they believe
developers should integrate it in their applications. This isn’t to say NewTek’s descriptions are
false but it reads like a company trying to tell you why their shovel will dig the specific hole you
need to dig better then others.
While physical switches exist they remain significantly more cost prohibitive then digital
solutions using NDI, so we will be focussing on solutions such as Open Broadcaster Software
(OBS) and Vmix which are widely used in the industry due to their robust feature set (4).
However, OBS requires a significant amount of technical expertise to set up and operate
effectively, limiting its accessibility to smaller organizations or individuals with less technical
knowledge. Proprietary applications like Vmix offer comprehensive functionality, but are
cost-prohibitive and require extensive training to use, making them unsuitable for smaller teams
or those with limited resources (3).
The gap in the literature points to a clear need for a new application that bridges the
divide between functionality and usability. An ideal solution would leverage the strengths of NDI
technology, offering low-latency streaming while integrating user-friendly features for video
switching and stream mixing. It would also utilize the data on transmission machines to lower
the number of hands needed to stream proficiently. This would enable smaller organizations to
produce professional-grade broadcasts without the need for large teams or significant financial
investment.
Developing a user-friendly, scalable esports production application could enable access to
high-quality broadcasting tools, allowing smaller teams and organizations to compete on a more
level playing field with larger entities. By reducing the technical and financial barriers to entry,
such an application could foster greater innovation and diversity within the esports broadcasting
industry. The existing literature underscores the need for a more accessible esports production
solution that combines the low-latency benefits of NDI technology with user-friendly features for
video switching and mixing. Addressing this need could have a significant impact on the
industry, enabling a wider range of organizations to produce professional-level broadcasts and
contribute to the continued growth of esports at the grassroots level.
<br> <br>
# Proposed Work <br>
1. Outline architecture in order to figure out the most efficient way to integrate input such as
video or game data into a mix of streams getting transmitted as one output.
2. Develop using C++ in order to create backend and frontend to house our functionality
when implementing the NDI features.
3. Features include real time feed switching based on stream manger input or automatic
game data reading, customizable commentator overlays, live data feeds from popular
esports, and support for concurrent streams on one output such as a multicam.
4. Our target audience will be smaller esports organizations as well as independent creators
in order to lower the barrier to entry, but larger companies looking to increase efficiency
and cut costs may find great use in our developments.
<br> <br>
# Project Plan <br>
Week 1-2: Project proposal, figure out requirements, initial architecture design.
Week 3-4: develop backend, integrate NDI SDK and feed management.
Week 5-6: Develop frontend, GUI for broadcasting and custom overlay management.
Week 7: Integrate game data to poll for real time updates to swap between streams.
Week 8: Testing/debugging and optimization.
Week 9: Run user tests and acquire feedback.
Week 10: Polish and delivery.
<br> <br>
# Conclusion <br>
This proposal for a NDI project to manage streams and produce quality broadcasts meets the
current need for a low budget, minimal setup option for smaller organizations that can promote
esports broadcasting availability for all. By utilizing NDI technology, we will be able to achieve
this goal and further esports broadcasting at all levels by lowering the barrier to entry normally
present in the broadcasting industry.
