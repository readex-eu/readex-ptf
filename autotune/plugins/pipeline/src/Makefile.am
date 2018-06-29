pipeline_LTLIBRARIES       = libptfpipeline.la
pipelinedir = ${plugindir}/pipeline

libptfpipeline_la_CXXFLAGS = ${autotune_plugin_base_cxxflags} \
                            -I$(top_srcdir)/autotune/plugins/pipeline/include

libptfpipeline_la_SOURCES  = autotune/plugins/pipeline/src/PipelinePlugin.cc
libptfpipeline_la_LDFLAGS  = ${autotune_plugin_base_ldflags} -version-info 1:0:0 -lboost_program_options \
                             -release ${PIPELINE_VERSION_MAJOR}.${PIPELINE_VERSION_MINOR}.${PIPELINE_REVISION}
