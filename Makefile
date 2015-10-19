post-checkout-purge:
	rm Eradication/LibGenericClasses.cpp interventions/*EModule.cpp interventions/SimpleAnalyzer.cpp utils/Coredump.cpp

release:
	cd jsonspirit; make -j 4 release; cd ..
	cd cajun; make -j 4 release; cd ..
	cd unittest; make -j 4 release; cd ..
	cd campaign; make -j 4 release; cd ..
	cd utils; make -j 4 release; cd ..
	cd Eradication; chmod u+x makelinks; make createlinks; make -j 4 release; cd ..

debug:
	cd jsonspirit; make -j 4 debug; cd ..
	cd cajun; make -j 4 debug; cd ..
	#cd unittest; make debug; cd ..
	cd campaign; make -j 4 debug; cd ..
	cd utils; make -j 4 debug; cd ..
	cd Eradication; make -j 4 debug; cd ..

clean:
	#cd unittest; make clean.dbg; make clean.rls; cd ..
	cd campaign; make clean.dbg; make clean.rls; cd ..
	cd utils; make clean.dbg; make clean.rls; cd ..
	cd Eradication; make clean.dbg; make clean.rls; cd ..
