#!/usr/bin/env python3
# Author: Sean Pesce

import argparse
import json
import sys



def obj_from_json_fpath(fpath):
    """
    Returns a Python dict or list parsed from JSON data in the file at the specified path
    """
    data = b''
    with open(fpath, 'rb') as f:
        data = f.read()
    return json.loads(data)



def get_textlist(data, id):
    if type(data) in (str, bytes, bytearray, memoryview):
        data = json.loads(data)
    if type(id) != int:
        id = int(id)
    
    for i in range(0, len(data['textlists'])):
        textlist = data['textlists'][i]
        if int(textlist['id']) == id:
            return textlist
    return None



def get_textlist_string(data, textlist_id, string_id):
    if type(data) in (str, bytes, bytearray, memoryview):
        data = json.loads(data)
    if type(textlist_id) != int:
        textlist_id = int(textlist_id)
    if type(string_id) != int:
        string_id = int(string_id)
    
    for i in range(0, len(data['textlists'])):
        textlist = data['textlists'][i]
        if int(textlist['id']) == textlist_id:
            for j in range(0, len(data['textlists'][i]['content'])):
                string_item = data['textlists'][i]['content'][j]
                if int(string_item['id']) == string_id:
                    return string_item
    return None



def analyze_textlists(main, other, ignore_modifications=False):
    if type(main) in (str, bytes, bytearray, memoryview):
        main = json.loads(main)
    if type(other) in (str, bytes, bytearray, memoryview):
        other = json.loads(other)
    results = dict()
    results['textlists'] = dict()
    results['textlists']['deleted'] = list()  # Entries that exist in main, but not other
    results['textlists']['new'] = list()  # Entries that exist in other, but not main
    if not ignore_modifications:
        results['textlists']['modified'] = dict()  # Entries that exist in both data sets, but contain different data
    
    main_textlist_items = dict()
    
    for i in range(0, len(main['textlists'])):
        textlist = main['textlists'][i]
        main_textlist_items[textlist['id']] = dict(textlist)
        main_textlist_items[textlist['id']]['strings'] = dict()
        
        other_textlist = get_textlist(other, textlist['id'])
        if other_textlist is None:
            results['textlists']['deleted'].append(textlist['id'])
        
        elif ignore_modifications:
            continue
        
        elif textlist != other_textlist:
            results['textlists']['modified'][textlist['id']] = dict()
            results['textlists']['modified'][textlist['id']]['deleted'] = list()
            results['textlists']['modified'][textlist['id']]['new'] = list()
            results['textlists']['modified'][textlist['id']]['modified'] = list()
            
            for j in range(0, len(textlist['content'])):
                string_item = textlist['content'][j]
                main_textlist_items[textlist['id']]['strings'][string_item['id']] = dict(string_item)
                other_string_item = get_textlist_string(other, other_textlist['id'], string_item['id'])
                
                if other_string_item is None:
                    results['textlists']['modified'][textlist['id']]['deleted'].append(string_item['id'])
                elif string_item != other_string_item:
                    results['textlists']['modified'][textlist['id']]['modified'].append(string_item['id'])
            
    
    # Check for new Textlists
    for i in range(0, len(other['textlists'])):
        textlist = other['textlists'][i]
        if textlist['id'] not in main_textlist_items:
            results['textlists']['new'].append(textlist['id'])
            continue
        
        if ignore_modifications or textlist['id'] not in results['textlists']['modified']:
            continue
        
        # Check for new strings in Textlist
        for j in range(0, len(textlist['content'])):
            string_item = textlist['content'][j]
            main_textlist = main_textlist_items[textlist['id']]
            if string_item['id'] not in main_textlist['strings']:
                results['textlists']['modified'][textlist['id']]['new'].append(string_item['id'])
    
    return results



def get_subtitle_resource(data, id):
    if type(data) in (str, bytes, bytearray, memoryview):
        data = json.loads(data)
    
    for i in range(0, len(data['subtitles'])):
        resource = data['subtitles'][i]
        if resource['id'] == id:
            return resource
    return None



def get_subtitle_video(data, res_id, vid_id):
    if type(data) in (str, bytes, bytearray, memoryview):
        data = json.loads(data)
    if type(vid_id) != int:
        vid_id = int(vid_id)
    
    for i in range(0, len(data['subtitles'])):
        resource = data['subtitles'][i]
        if resource['id'] == res_id:
            for j in range(0, len(data['subtitles'][i]['content'])):
                vid_item = data['subtitles'][i]['content'][j]
                if int(vid_item['video_id']) == vid_id:
                    return vid_item
    return None



def analyze_subtitles(main, other, ignore_modifications=False):
    if type(main) in (str, bytes, bytearray, memoryview):
        main = json.loads(main)
    if type(other) in (str, bytes, bytearray, memoryview):
        other = json.loads(other)
    results = dict()
    results['subtitles'] = dict()
    results['subtitles']['deleted'] = list()  # Entries that exist in main, but not other
    results['subtitles']['new'] = list()  # Entries that exist in other, but not main
    #if not ignore_modifications:
    results['subtitles']['modified'] = dict()  # Entries that exist in both data sets, but contain different data
    
    main_resource_items = dict()
    
    for i in range(0, len(main['subtitles'])):
        resource = main['subtitles'][i]
        main_resource_items[resource['id']] = dict(resource)
        main_resource_items[resource['id']]['videos'] = dict()
        
        other_resource = get_subtitle_resource(other, resource['id'])
        if other_resource is None:
            results['subtitles']['deleted'].append(resource['id'])
        
        elif resource != other_resource:
            results['subtitles']['modified'][resource['id']] = dict()
            results['subtitles']['modified'][resource['id']]['deleted'] = list()
            results['subtitles']['modified'][resource['id']]['new'] = list()
            #if not ignore_modifications:
            results['subtitles']['modified'][resource['id']]['modified'] = dict()
            
            for j in range(0, len(resource['content'])):
                video_item = resource['content'][j]
                main_resource_items[resource['id']]['videos'][video_item['video_id']] = dict(video_item)
                #main_resource_items[resource['id']]['videos'][video_item['video_id']]['subs'] = dict()
                other_video_item = get_subtitle_video(other, other_resource['id'], video_item['video_id'])
                
                if other_video_item is None:
                    results['subtitles']['modified'][resource['id']]['deleted'].append(video_item['video_id'])
                
                #elif ignore_modifications:
                #    continue
                
                elif video_item != other_video_item:
                    results['subtitles']['modified'][resource['id']]['modified'][video_item['video_id']] = dict()
                    results['subtitles']['modified'][resource['id']]['modified'][video_item['video_id']]['deleted'] = 0
                    results['subtitles']['modified'][resource['id']]['modified'][video_item['video_id']]['new'] = 0
                    results['subtitles']['modified'][resource['id']]['modified'][video_item['video_id']]['modified'] = list()
                    
                    if len(video_item['subs']) < len(other_video_item['subs']):
                        # Lines were added to the new data set
                        results['subtitles']['modified'][resource['id']]['modified'][video_item['video_id']]['new'] = len(other_video_item['subs']) - len(video_item['subs'])
                        if ignore_modifications:
                            continue
                        
                        modified_lines = [line for line in video_item['subs'] if line not in other_video_item['subs']]
                        for line in modified_lines:
                            results['subtitles']['modified'][resource['id']]['modified'][video_item['video_id']]['modified'].append(line['start'])
                    
                    elif len(video_item['subs']) > len(other_video_item['subs']):
                        # Lines are missing from the new data set
                        results['subtitles']['modified'][resource['id']]['modified'][video_item['video_id']]['deleted'] = len(video_item['subs']) - len(other_video_item['subs'])
                        if ignore_modifications:
                            continue
                        
                        modified_lines = [line for line in video_item['subs'] if line not in other_video_item['subs']]
                        # If the number of unmatched lines is greater than the number of deleted lines, some remaining lines were modified
                        modified_count = len(modified_lines) - results['subtitles']['modified'][resource['id']]['modified'][video_item['video_id']]['deleted']
                        # @TODO: Heuristics to determine which (if any) subtitle entries were modified when some lines were deleted? Currently we just take the first N
                        for mod_i in range(0, modified_count):
                            results['subtitles']['modified'][resource['id']]['modified'][video_item['video_id']]['modified'].append(modified_lines[mod_i]['start'])
                
                    elif ignore_modifications:
                        continue
                    
                    else:
                        # Same number of lines in both data sets; check if any were edited
                        modified_lines = [line for line in video_item['subs'] if line not in other_video_item['subs']]
                        for line in modified_lines:
                            results['subtitles']['modified'][resource['id']]['modified'][video_item['video_id']]['modified'].append(line['start'])
    
    # Check for new resource containers
    for i in range(0, len(other['subtitles'])):
        resource = other['subtitles'][i]
        if resource['id'] not in main_resource_items:
            results['subtitles']['new'].append(resource['id'])
            continue
        
        if resource['id'] not in results['subtitles']['modified']:
            continue
        
        # Check for new videos in resource container
        for j in range(0, len(resource['content'])):
            video_item = resource['content'][j]
            main_resource = main_resource_items[resource['id']]
            if video_item['video_id'] not in main_resource['videos']:
                results['subtitles']['modified'][resource['id']]['new'].append(video_item['video_id'])
    
    return results



if __name__ == '__main__':
    argparser = argparse.ArgumentParser(
                    prog=sys.argv[0],
                    description='Analyzes and compares language translation data sets for Sean P\'s DXMD Language Translation Framework to enumerate differences.',
                    epilog='')  # Text at the bottom of help output
    
    arg_action_textlists = ['textlists']#, 'textlist', 't', 'tl']
    arg_action_subtitles = ['subtitles']#, 'subtitle', 'subs', 'sub', 's', 'st']
    arg_action_all = ['all']#, 'a']
    argparser.add_argument('action', action='store', choices=arg_action_all+arg_action_textlists+arg_action_subtitles, type=str.lower, help='Type of analysis to run')
    argparser.add_argument('main_data_set_fpath', action='store', help='File path for the authoritative data set (file contents should be in JSON format)')
    argparser.add_argument('other_data_set_fpath', action='store', help='File path for the new/other/modified data set (file contents should be in JSON format)')
    
    argparser.add_argument('-v', '--verbose', action='store_true')
    argparser.add_argument('-m', '--ignore-modifications', action='store_true', help='Ignore modified entries (can be useful when comparing data sets of different languages)')
    
    args = argparser.parse_args()
    
    main_data_set = obj_from_json_fpath(args.main_data_set_fpath)
    other_data_set = obj_from_json_fpath(args.other_data_set_fpath)
    
    results = dict()
    summary = f'\nStatistics for {args.other_data_set_fpath}:'
    if args.action in arg_action_textlists or args.action in arg_action_all:
        results = analyze_textlists(main_data_set, other_data_set, ignore_modifications=args.ignore_modifications)
        summary += f'\n  Textlists:\n    New: {len(results["textlists"]["new"])}\n    Deleted: {len(results["textlists"]["deleted"])}'
        if not args.ignore_modifications:
            summary += f'\n    Modified: {len(results["textlists"]["modified"])}'
    
    if args.action in arg_action_subtitles or args.action in arg_action_all:
        results['subtitles'] = analyze_subtitles(main_data_set, other_data_set, ignore_modifications=args.ignore_modifications)['subtitles']
        summary += f'\n  Subtitle containers:\n    New: {len(results["subtitles"]["new"])}\n    Deleted: {len(results["subtitles"]["deleted"])}'
        #if not args.ignore_modifications:
        summary += f'\n    Modified: {len(results["subtitles"]["modified"])}'
        
        video_modify_counts = {'new':0,'deleted':0,'modified':0}
        subtitle_modify_counts = {'new':0,'deleted':0,'modified':0}
        for res_id in results['subtitles']['modified'].keys():
            video_modify_counts['new'] += len(results['subtitles']['modified'][res_id]['new'])
            video_modify_counts['deleted'] += len(results['subtitles']['modified'][res_id]['deleted'])
            #if not args.ignore_modifications:
            video_modify_counts['modified'] += len(results['subtitles']['modified'][res_id]['modified'])
            for video_id in results['subtitles']['modified'][res_id]['modified']:
                subtitle_modify_counts['new'] += results['subtitles']['modified'][res_id]['modified'][video_id]['new']
                subtitle_modify_counts['deleted'] += results['subtitles']['modified'][res_id]['modified'][video_id]['deleted']
                if not args.ignore_modifications:
                    subtitle_modify_counts['modified'] += len(results['subtitles']['modified'][res_id]['modified'][video_id]['modified'])
        summary += f'\n  Subtitle videos:\n    New: {video_modify_counts["new"]}\n    Deleted: {video_modify_counts["deleted"]}'
        #if not args.ignore_modifications:
        summary += f'\n    Modified: {video_modify_counts["modified"]}'
        
        summary += f'\n  Subtitle lines:\n    New: {subtitle_modify_counts["new"]}\n    Deleted: {subtitle_modify_counts["deleted"]}'
        if not args.ignore_modifications:
            summary += f'\n    Modified: {subtitle_modify_counts["modified"]}'  # @TODO: Need heuristics to determine which (if any) subtitle entries were modified when some lines were deleted
        
    
    print(json.dumps(results, indent=2, ensure_ascii=True))
    print(summary, file=sys.stderr)
    
    

