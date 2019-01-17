all: rect.vert.spv rect.frag.spv rect.geom.spv rect_uni.vert.spv rect_inst.vert.spv

%.vert.spv: %.vert
	glslangValidator -V $*.vert -o $*.vert.spv

%.frag.spv: %.frag
	glslangValidator -V $*.frag -o $*.frag.spv

%.geom.spv: %.geom
	glslangValidator -V $*.geom -o $*.geom.spv

clean:
	rm -rf *.spv
