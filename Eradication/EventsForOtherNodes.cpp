/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/
#include "stdafx.h"
#include "EventsForOtherNodes.h"
#include "IArchive.h"
#include "Sugar.h"


namespace Kernel
{
    EventsForOtherNodes::EventsForOtherNodes()
        : m_NodeEventMap()
    {
    }

    EventsForOtherNodes::~EventsForOtherNodes()
    {
        Clear();
    }


    bool EventsForOtherNodes::operator==( const EventsForOtherNodes& rThat ) const
    {
        if( this->m_NodeEventMap.size() != rThat.m_NodeEventMap.size() ) return false ;

        std::map<suids::suid,std::vector<std::string>>::const_iterator this_it = this->m_NodeEventMap.begin();
        std::map<suids::suid,std::vector<std::string>>::const_iterator that_it = rThat.m_NodeEventMap.begin();
        while( this_it != this->m_NodeEventMap.end() )
        {
            if( (*this_it).first != (*that_it).first ) return false ;

            if( (*this_it).second.size() != (*that_it).second.size() ) return false ;

            for( int i = 0 ; i < (*this_it).second.size() ; i++ )
            {
                if( (*this_it).second[i] != (*that_it).second[i] ) return false ;
            }

            this_it++ ;
            that_it++ ;
        }
        return true ;
    }

    bool EventsForOtherNodes::operator!=( const EventsForOtherNodes& rThat ) const
    {
        return !operator==( rThat );
    }

    void EventsForOtherNodes::Add( const suids::suid& rNodeSuid, const std::string& rEventName )
    {
        m_NodeEventMap[ rNodeSuid ].push_back( rEventName );
    }

    const std::vector<std::string>& EventsForOtherNodes::GetEventNames( const suids::suid& rNodeSuid )
    {
        return m_NodeEventMap[ rNodeSuid ] ;
    }

    void EventsForOtherNodes::Clear()
    {
        m_NodeEventMap.clear();
    }

    void EventsForOtherNodes::Print()
    {
        for( auto entry : m_NodeEventMap )
        {
            auto& dest_node_id = entry.first;
            auto& event_name_list = entry.second;
            std::ostringstream ss ;
            ss << dest_node_id.data  ;
            for( auto event_name : event_name_list )
            {
                ss << ", " << event_name ;
            }
            printf("Rank=%2d: %s\n",EnvPtr->MPI.Rank,ss.str().c_str()); fflush(stdout);
        }
    }

    void EventsForOtherNodes::Update( const EventsForOtherNodes& rEfon )
    {
        for( auto entry : rEfon.m_NodeEventMap )
        {
            auto& dest_node_id = entry.first;
            auto& event_name_list = entry.second;
            for( auto event_name : event_name_list )
            {
                m_NodeEventMap[ dest_node_id ].push_back( event_name );
            }
        }
    }

    void EventsForOtherNodes::serialize( IArchive& ar, EventsForOtherNodes& obj )
    {
        ar & obj.m_NodeEventMap;
    }
}
