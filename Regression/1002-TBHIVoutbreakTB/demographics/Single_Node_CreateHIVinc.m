function output_json = Single_Node_CreateHIVinc(json_template, input_file_m,input_file_f,outfile_name)

Tmp_json = loadJson(json_template);


% do some processing to create 3d matrix, check that the data was read in
% correctly

tmp_male = csvread(input_file_m);  %should be a csv file
tmp_female = csvread(input_file_f); %should be a csv file

male_data = tmp_male(4:end,:);
female_data = tmp_female(4:end,:);

%check that number of columns rows, start values are the same between men
%and women (later can change if we want different age groups for each
%gender but won't work as is

male_compare = tmp_male(1:3,1:2);
female_compare = tmp_female(1:3,1:2);

if (male_compare ~= female_compare)
    
    Error('Female and male files do not agree in dimension');
    
end



 
numcols = male_compare(1,1);
numrows = male_compare(1,2);
start_val_col = male_compare(2,1);
col_incr = male_compare(2,2);
start_val_row = male_compare(3,1);
row_incr = male_compare(3,2);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

row_vals = start_val_row * ones(numrows,1) + row_incr * [0:(numrows-1)]';
col_vals = start_val_col * ones(numcols,1) + col_incr * [0:(numcols-1)]';


%create three d matrix 

Three_d_matx(1,:,:) = male_data';
Three_d_matx(2,:,:) = female_data';
%do for single mode only

if (numel(Tmp_json.Nodes) > 1 )
    warning('Json warning','More than one Node, only first node will be se, see tempalte %s', json_template);
end


Node_Coinfection = Tmp_json.Nodes{1}.IndividualAttributes.HIVCoinfectionDistribution;

Node_Coinfection.NumPopulationGroups = [2,numel(row_vals),numel(col_vals)];
Node_Coinfection.PopulationGroups{1} = {0, 1};
Node_Coinfection.PopulationGroups{2} = row_vals';
Node_Coinfection.PopulationGroups{3} = col_vals';

Node_Coinfection.AxisNames = {'gender', 'time', 'age'};
Node_Coinfection.AxisUnits = {'male =0, female=1', 'days', 'years'};

for ii = 1:2 %only have two genders
    
    for jj =1:numel(row_vals)
        
        for kk =1:numel(col_vals)
            
          
         Node_Coinfection.ResultValues{ii}{jj}{kk} =  Three_d_matx(ii,jj,kk);
         
        end
    end
end
            
       Tmp_json.Nodes{1}.IndividualAttributes.HIVCoinfectionDistribution = Node_Coinfection;
       output_json = Tmp_json;
       
       saveJson(outfile_name,output_json);
       
       
     
    
    


