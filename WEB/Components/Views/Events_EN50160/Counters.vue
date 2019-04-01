
             WeekStats = [];
             if(result['Starttime'] != null) {
               if(WeekStats[0] === undefined) InitWeek(0, new Date(selected_year, 0, 1, 0, 0, 0, 0), new Date(selected_year, 11, 31, 23, 59, 59, 999));
               for(i = 0; i < result['Starttime'].length; i++) {
                 var Week = GetWeekOfYear(result['Starttime'][i]);
                 if(WeekStats[Week] === undefined) {
                   var Start = GetDateOfISOWeek(Week, selected_year);
                   var End = new Date(Start.getFullYear(), Start.getMonth(), Start.getDate() + 6);
                   InitWeek(Week, Start, End);
                 }
                 var Duration = result['Duration'][i] / Nodes['Devices'][Node]['SampleFreq'];
                 var Percent = result['Percentage'][i];
                 if((Duration < 60) && ((Percent < -15) || (Percent > -10))) {
                   WeekStats[Week].Dip = WeekStats[Week].Dip + 1;
                   WeekStats[0].Dip = WeekStats[0].Dip + 1;
                 }
                 if((Duration < 180) && (Percent < -99)) {
                   WeekStats[Week].Short = WeekStats[Week].Short + 1;
                   WeekStats[0].Short = WeekStats[0].Short + 1;
                 }
                 if((Duration > 180) && (Percent < -99)) {
                   WeekStats[Week].Long = WeekStats[Week].Long + 1;
                   WeekStats[0].Long = WeekStats[0].Long + 1;
                 }
               }
             }
             GetStatsData(selected_year);
