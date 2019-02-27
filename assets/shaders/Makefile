SHADERS := sprite.vert.spv \
           sprite.frag.spv

all: $(SHADERS)

%.vert.spv: %.vert
	glslangValidator -V $*.vert -o $*.vert.spv

%.frag.spv: %.frag
	glslangValidator -V $*.frag -o $*.frag.spv

%.geom.spv: %.geom
	glslangValidator -V $*.geom -o $*.geom.spv

clean:
	rm -rf *.spv
