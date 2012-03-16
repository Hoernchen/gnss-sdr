/*!
 * \file gps_l1_ca_tcp_connector_tracking.cc
 * \brief Implementation of an adapter of a TCP connector block based on code DLL + carrier PLL
 * \author David Pubill, 2012. dpubill(at)cttc.es
 *         Javier Arribas, 2011. jarribas(at)cttc.es
 *
 * Code DLL + carrier PLL according to the algorithms described in:
 * K.Borre, D.M.Akos, N.Bertelsen, P.Rinder, and S.H.Jensen,
 * A Software-Defined GPS and Galileo Receiver. A Single-Frequency
 * Approach, Birkhauser, 2007
 *
 * -------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2012  (see AUTHORS file for a list of contributors)
 *
 * GNSS-SDR is a software defined Global Navigation
 *          Satellite Systems receiver
 *
 * This file is part of GNSS-SDR.
 *
 * GNSS-SDR is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * at your option) any later version.
 *
 * GNSS-SDR is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNSS-SDR. If not, see <http://www.gnu.org/licenses/>.
 *
 * -------------------------------------------------------------------------
 */

#include "gps_l1_ca_tcp_connector_tracking.h"
#include "GPS_L1_CA.h"
#include "configuration_interface.h"
#ifdef GNSS_SDR_USE_BOOST_ROUND
  #include <boost/math/special_functions/round.hpp>
#endif
#include <gnuradio/gr_io_signature.h>
#include <glog/log_severity.h>
#include <glog/logging.h>

using google::LogMessage;

GpsL1CaTcpConnectorTracking::GpsL1CaTcpConnectorTracking(
        ConfigurationInterface* configuration, std::string role,
        unsigned int in_streams, unsigned int out_streams,
        gr_msg_queue_sptr queue) :
        role_(role), in_streams_(in_streams), out_streams_(out_streams),
        queue_(queue)
{

    DLOG(INFO) << "role " << role;
    //DLOG(INFO) << "vector length " << vector_length;

    //################# CONFIGURATION PARAMETERS ########################

    int fs_in;
    int vector_length;
    int f_if;
    bool dump;
    std::string dump_filename;
    std::string item_type;
    std::string default_item_type = "gr_complex";
    float pll_bw_hz;
    float dll_bw_hz;
    float early_late_space_chips;
    size_t port_ch0;

    item_type = configuration->property(role + ".item_type",default_item_type);
    //vector_length = configuration->property(role + ".vector_length", 2048);
    fs_in = configuration->property("GNSS-SDR.internal_fs_hz", 2048000);
    f_if = configuration->property(role + ".if", 0);
    dump = configuration->property(role + ".dump", false);
    pll_bw_hz = configuration->property(role + ".pll_bw_hz", 50.0);
    dll_bw_hz = configuration->property(role + ".dll_bw_hz", 2.0);
    early_late_space_chips = configuration->property(role + ".early_late_space_chips", 0.5);
    port_ch0 = configuration->property(role + ".port_ch0", 2060);

    std::string default_dump_filename = "./track_ch";
    dump_filename = configuration->property(role + ".dump_filename",
            default_dump_filename); //unused!
#ifdef GNSS_SDR_USE_BOOST_ROUND
    vector_length = round(fs_in / (GPS_L1_CA_CODE_RATE_HZ / GPS_L1_CA_CODE_LENGTH_CHIPS));
#else
    vector_length = std::round(fs_in / (GPS_L1_CA_CODE_RATE_HZ / GPS_L1_CA_CODE_LENGTH_CHIPS));
#endif
    //################# MAKE TRACKING GNURadio object ###################
    if (item_type.compare("gr_complex") == 0)
        {
            item_size_ = sizeof(gr_complex);
            tracking_ = gps_l1_ca_tcp_connector_make_tracking_cc(
            		f_if,
                    fs_in,
                    vector_length,
                    queue_,
                    dump,
                    dump_filename,
                    pll_bw_hz,
                    dll_bw_hz,
                    early_late_space_chips,
                    port_ch0);
        }
    else
        {
            LOG_AT_LEVEL(WARNING) << item_type << " unknown tracking item type.";
        }

    DLOG(INFO) << "tracking(" << tracking_->unique_id() << ")";
}

GpsL1CaTcpConnectorTracking::~GpsL1CaTcpConnectorTracking()
{
}

void GpsL1CaTcpConnectorTracking::start_tracking()
{
    tracking_->start_tracking();
}

/*
 * Set tracking channel unique ID
 */
void GpsL1CaTcpConnectorTracking::set_channel(unsigned int channel)
{
    channel_ = channel;
    tracking_->set_channel(channel);
}

/*
 * Set tracking channel internal queue
 */
void GpsL1CaTcpConnectorTracking::set_channel_queue(
        concurrent_queue<int> *channel_internal_queue)
{
    channel_internal_queue_ = channel_internal_queue;

    tracking_->set_channel_queue(channel_internal_queue_);

}

void GpsL1CaTcpConnectorTracking::set_gnss_synchro(Gnss_Synchro* p_gnss_synchro)
{
    tracking_->set_gnss_synchro(p_gnss_synchro);
}

void GpsL1CaTcpConnectorTracking::connect(gr_top_block_sptr top_block)
{
    //nothing to connect, now the tracking uses gr_sync_decimator
}

void GpsL1CaTcpConnectorTracking::disconnect(gr_top_block_sptr top_block)
{
    //nothing to disconnect, now the tracking uses gr_sync_decimator
}

gr_basic_block_sptr GpsL1CaTcpConnectorTracking::get_left_block()
{
    return tracking_;
}

gr_basic_block_sptr GpsL1CaTcpConnectorTracking::get_right_block()
{
    return tracking_;
}

