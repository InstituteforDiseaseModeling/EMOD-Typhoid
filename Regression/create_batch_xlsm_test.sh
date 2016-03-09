

for file in `find Scenarios/STIAndHIV/05_Intervention_Examples/ -name param_overrides.json|sed 's/param_overrides.json//g'`; do 
    echo "python y:\Scripts\xslm2json.py " $file/config.xlsm; 
    echo "python .\extract_overrides.py defaults\hiv_scenario5_default_config.json .config_from_excel.json > " $file\param_overrides.json 
    echo "del .config_from_excel.json "
done > ./batch_convert_xlsm_to_json.bat

