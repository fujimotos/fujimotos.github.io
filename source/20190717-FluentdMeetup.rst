:orphan:

==========================================
The State of Windows Support in Fluent Bit
==========================================

:Author: Fujimoto Seiji
:Published: 2019-07-17
:Copyright: This document has been placed in the public domain.

This is my talk given on July 17 2019 at Fluentd Meetup,
a collocated event of `Open Source Summit Tokyo 2019 <https://events.linuxfoundation.org/events/open-source-summit-japan-2019/>`_

Many thanks to Eduardo Silva and Masahiro Nakagawa for inviting me to talk.

1. Introduction
---------------


.. list-table::

 - * .. figure:: 20190717/slide-thumb-01.png
   *  I'm an engineer from ClearCode Inc. and right now working on the project to port Fluent Bit to Windows.

      Today I want to provide a general introduction of Fluent Bit and talk about the state of Windows support I'm working on. 

2. Log management problem
-------------------------

.. list-table::

 - * .. figure:: 20190717/slide-thumb-03.png
   * Let me start by saying that we have tons and tons of log data to manage,
     We have logs from app servers, logs from database servers, logs from
     proxy servers and so on.

     And the amount of logs is increasing very rapidly each year.

 - * .. figure:: 20190717/slide-thumb-04.png
   * This is a traditional web application, which has two NGINX servers,
     two Python app servers and two PostgreSQL servers.

     The question is **how many log files this stack contains?**
     Let me count them up.
     First we have two access logs from NGINX of course, we have two app
     logs from Python application and two database logs from PostgreSQL.

 - * .. figure:: 20190717/slide-thumb-05.png
   * So we have at least six log files, all of which we need to monitor carefully,
     or we'll miss serious incidents like connection limit exceeded,
     exception occurred and database replication failed etc.

     It's already not a quite trivial task to monitor them properly.
     But things get better.

 - * .. figure:: 20190717/slide-thumb-06.png
   * It's age of micro services and virtualisation. Now you can
     literally have a dozen of severs just to run a single application,
     and every server emits log files independently with each other.

     In this slide now you have thirty six logs to manage, since you have six
     micro applications and each emits six logs.

     How do you monitor them?

 - * .. figure:: 20190717/slide-thumb-07.png
   * It's no wonder that everyone suddenly rushes to log management solutions
     like Splunk, Elasticsearch, time series database like InfluxDB and
     message queues like Apache Kafka.

     I'm sure that you are using one or two of them at work. I for one use
     them at work. They are indeed amazing products.

 - * .. figure:: 20190717/slide-thumb-08.png
   * But the remaining problem here is *"how do you send your logs to these products?"*
     Your log files do not transfer themselves to other servers, so you need to make it happen.

     The question is "how".

 - * .. figure:: 20190717/slide-thumb-09.png
   * I summarize my points so far.

     You have a large amount of log data at one hand, and  you have amazing log management solutions at another hand. What is missing is a pipeline to connect them.

3. Fluent Bit as a Solution
---------------------------

.. list-table::

 - * .. figure:: 20190717/slide-thumb-10.png
   * We develop Fluent Bit to solve this problem.

     Fluent Bit is a program that can talk a number of protocols and let you transfer

 - * .. figure:: 20190717/slide-thumb-11.png
   * For example, it can speak Kafka protocol, so you can use Fluent Bit as a pipe to connect your data source to a Kafka cluster. Also it can speak Elasticsearch protocol, so you can ship logs into your Elasticsearch server.

     Fluent Bit supports more than twenty types of outputs, and twenty types of inputs. Thus, it is pretty much capable.

 - * .. figure:: 20190717/slide-thumb-12.png
   * At this point, you may wonder "Isn't that what Fluentd exactly does?  What is the difference with Fluentd?"

     The keyword is "efficiency".

     As you may know, Fluentd is written in Ruby. It's great and very convenient language but quite slower to run than compiled languages like C.  Since Fluent Bit is written in C, it can process a lot more data efficiently than Fluentd.

 - * .. figure:: 20190717/slide-thumb-13.png
   * This table illustrates the difference between them. Blue is Fluent Bit, gray is Fluentd and Y-axis is the lines processed per second (the higher, the better).

     Talking about LTSV, Fluent Bit can process two times more data per second, so pretty much faster than Fluentd.

     Even with JSON, which Fluentd has a quite optimized parser for, but still Fluent Bit outperforms Fluentd by twenty or thirty percent.

 - * .. figure:: 20190717/slide-thumb-14.png
   * The summary so far is:

     - Fluent Bit is a very convenient log shipper.
     - It can transfer your logs to a number of services.
     - It's very efficient because of written in C.

4. State of Windows Support
---------------------------

.. list-table::

 - * .. figure:: 20190717/slide-thumb-15.png
   * So this is the main part.

     First I'd like to talk a bit about the history of Fluent Bit.
     
     I'd like to start by telling that Fluent Bit was initially developed only for Linux, indeed only for embedded Linux at first.
     
 - * .. figure:: 20190717/slide-thumb-16.png
   * Five years ago, in the initial commit of Fluent Bit, the readme said "Fluent Bit is a collection tool designed for Embedded Linux that collects Kernel messages and Hardware metrics".

     So that was the start. It was meant to be for Embedded Linux.

 - * .. figure:: 20190717/slide-thumb-17.png
   * Since then, we got full Linux support, OSX support and BSD support in 2015.
     We have also expand the supported architectures. Notably we have now full
     support for ARM 64 since two years ago.
     
     And this year we've gotten Fluent Bit to run on Windows.

 - * .. figure:: 20190717/slide-thumb-18.png
   * So what is the current status of Windows support? The porting project started the last December, so we have roughly six month's work.

     It actually came along very smoothly. We've done porting most of core engine, and around third of plugins are working.

     It's still quite an early stage, but the Windows port is already usable.

 - * .. figure:: 20190717/slide-thumb-19.png
   * More specifically, the works we've done are: we made it possible to compile Fluent Bit on MSVC, and enabled to launch Fluent Bit from PowerShell.

     Also we've done porting twenty plugins to Windows, and installers are also available in two flavours: EXE and ZIP, which make installation pretty much trivial.

5. Tutorial for Windows Users
-----------------------------

.. list-table::

 - * .. figure:: 20190717/slide-thumb-20.png
   * So I want to explain how you can use Fluent Bit today.

     This is very easy indeed, so I'd like to show how to do that in a step-by-step manner.

 - * .. figure:: 20190717/slide-thumb-21.png
   * Let me first set up some context.

     Suppose you have some application running on Windows and the application outputs its logs to :file:`C:\\log\\app.log`, and you also have Kafka cluster as a central message bus.
          
     The task is to transfer the log file from Windows to Kafka.
          
     Using Fluent Bit, we can set up such a pipeline just in three steps.

 - * .. figure:: 20190717/slide-thumb-22.png
   * First, download the ZIP installer. We have EXE and ZIP installers as I said, and ZIP is easier for quick testing, so I use the one here.

     We build installers for each commit on AppVeyor. You can download from there if you want the latest one.

     Open the link on your browser and just click and save it.

 - * .. figure:: 20190717/slide-thumb-23.png
   * Next, expand the ZIP archive.

     You need to just click the ZIP archive and select "Expand All" on Explorer or you can use the "Expand-Archive" commandlet on PowerShell.

     Both work fine, so choose the one you like.

 - * .. figure:: 20190717/slide-thumb-24.png
   * Then open PowerShell and just launch Fluent Bit.

     The ZIP file contains fluent-bit.exe which is a stand-alone binary.  As you can see in the slide, it is included in the bin directory

 - * .. figure:: 20190717/slide-thumb-25.png
   * As I said, the target log file is stored in :file:`C:\\logs\\app.log`.

     We use tail plugin as an input, and set the path accordingly.

 - * .. figure:: 20190717/slide-thumb-26.png
   * And we assumed that the output is Apache Kafka.

     We use the Kafka output plugin, set broker's IP address and tell Fluent Bit to send the data with "windows" topic

     That's it! You should see log lines running to your Kafka server.

     Alternatively, you can create a configuration file and feed it to Fluent Bit via `-c` option. You can download a sample configuration file here.


 - * .. figure:: 20190717/slide-thumb-27.png
   * The summary is:

     - Download the ZIP archive.
     - Unzip it.
     - Run the executable.

     This is all that you need. Please try at home if you have some interest in Fluent Bit, and please give us some feedback.

6. Future of Fluent Bit
-----------------------

.. list-table::

 - * .. figure:: 20190717/slide-thumb-28.png
   * So this is status of our efforts to make Fluent Bit Windows-compatible log shipping solution.

     I'd like to finish my talk with sharing some ideas of future development.

 - * .. figure:: 20190717/slide-thumb-29.png
   * First I'm planning to port the remaining thirty two plugins to Windows as much as possible.

     As to the plugin, we are right now working on a plugin for Windows Event Log which should be useful for Windows administrators.

     And I'm planning to add more documents, since it's kinda sparse right now.

 - * .. figure:: 20190717/slide-thumb-30.png
   * And we need feedbacks from users!

     What is your use case? Which service do you want to connect to?  If you have idea please tell us on the issue tracker on GitHub.  I'd like to hear from you.

     Thank you.
