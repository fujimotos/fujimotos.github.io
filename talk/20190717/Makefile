IMAGES = img/overview.png img/comp.png img/kafka.png img/lamp.png img/lamp-log.png img/lamp-conn.png

all: slide.pdf webimg

slide.pdf: slide.html $(IMAGES)
	wkhtmltopdf -O Landscape $< $@

img/comp.png: dat/comp.gpi dat/comp.dat
	gnuplot dat/comp.gpi > $@

img/%.png: dat/%.dia
	dia -e $@ $^

webimg: slide.pdf
	pdftocairo -png -r 35 $^ img/slide-thumb
	pdftocairo -png -r 100 $^ img/slide

clean:
	rm -f slide.pdf
	rm -f $(IMAGES)
	rm -f img/slide*

.PHONY: clean all webimg
