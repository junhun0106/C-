set PATH=c:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Utilities\bin\x86\
fxc /Od /Zi /Ges /T vs_5_0 /E VSParticleStreamOut_Blood /Fo VSParticleStreamOut_Blood.cso Effect.fx
fxc /Od /Zi /Ges /T gs_5_0 /E GSParticleStreamOut_Blood /Fo GSParticleStreamOut_Blood.cso Effect.fx
fxc /Od /Zi /Ges /T vs_5_0 /E VSParticleDraw_Blood /Fo VSParticleDraw_Blood.cso Effect.fx
fxc /Od /Zi /Ges /T gs_5_0 /E GSParticleDraw_Blood /Fo GSParticleDraw_Blood.cso Effect.fx
fxc /Od /Zi /Ges /T ps_5_0 /E PSParticleDraw_Blood /Fo PSParticleDraw_Blood.cso Effect.fx
fxc /Od /Zi /Ges /T cs_5_0 /E HorzBlurCS /Fo HorzBlurCS.cso Effect.fx
fxc /Od /Zi /Ges /T cs_5_0 /E VertBlurCS /Fo VertBlurCS.cso Effect.fx