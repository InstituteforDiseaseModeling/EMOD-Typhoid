#!/usr/bin/python

"""
This module centralizes some small bits of functionality for SFT tests
to make sure we are using the same strings for messages and filenames.
"""

sft_output_filename = "scientific_feature_report.txt"

def format_success_msg( success ):
    report_msg = "SUMMARY: Success={0}\n".format( success )
    return report_msg 
