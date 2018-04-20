/* -*- c++ -*- */
/*
 * Copyright 2018 V3 Inc.
 *
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */
#include <boost/assign.hpp>
#include <iostream>

#include "arg_helpers.h"
#include "osmosdr/source.h"
#include "yunsdr_source_c.h"

using namespace boost::assign;

yunsdr_source_c_sptr make_yunsdr_source_c(const std::string &args)
{
  return gnuradio::get_initial_sptr(new yunsdr_source_c(args));
}

static size_t parse_nchan(const std::string &args)
{
  size_t nchan = 1;

  dict_t dict = params_to_dict(args);

  if (dict.count("nchan"))
    nchan = boost::lexical_cast< size_t >( dict["nchan"] );

  return nchan;
}

yunsdr_source_c::yunsdr_source_c(const std::string &args) :
    gr::hier_block2("yunsdr_source_c",
                   gr::io_signature::make(0, 0, 0),
                   gr::io_signature::make(parse_nchan(args),
                       parse_nchan(args),
                       sizeof(gr_complex)))
{
  uri = "192.168.1.10";
  frequency = 434000000;
  samplerate = 2500000;
  bandwidth = 2000000;
  buffer_size = 0x4000;
  gain_auto = false;
  gain_value = 50;
  filter = "";
  filter_auto = true;
  _freq_corr = 0.0;
  size_t nchan = 1;

  dict_t dict = params_to_dict(args);

  if (dict.count("nchan"))
    nchan = boost::lexical_cast< size_t >( dict["nchan"] );

  if (0 == nchan)
    nchan = 1;

  if (dict.count("uri"))
    uri = boost::lexical_cast< std::string >( dict["uri"] );

  std::cerr << "Using YunSDR URI = " << uri << std::endl;

  _src = gr::yunsdr::yunsdr_source::make(uri, frequency, samplerate,
                                     bandwidth, true, nchan>1?true:false,
                                     "manual", gain_value,
                                     "manual", gain_value,
                                     "internal", "auxdac1",
                                     buffer_size, "RX",
                                     filter.c_str(), filter_auto);

  for ( size_t i = 0; i < nchan; i++ )
    connect( _src, i, self(), i );
}

yunsdr_source_c::~yunsdr_source_c()
{
}

std::vector< std::string > yunsdr_source_c::get_devices( bool fake )
{
  std::vector< std::string > devices;

  if ( fake )
  {
    std::string args = "yunsdr,label='YunSDR'";
    args += ",uri=192.168.1.10";

    devices.push_back( args );
  }

  return devices;
}

std::string yunsdr_source_c::name()
{
  return "YunSDR";
}

size_t yunsdr_source_c::get_num_channels()
{
  return output_signature()->max_streams();
}

osmosdr::meta_range_t yunsdr_source_c::get_sample_rates( void )
{
  osmosdr::meta_range_t rates;

  rates += osmosdr::range_t( 2500000 );
  rates += osmosdr::range_t( 5000000 );
  rates += osmosdr::range_t( 10000000 );
  rates += osmosdr::range_t( 20000000 );

  return rates;
}

double yunsdr_source_c::set_sample_rate( double rate )
{
  samplerate = (unsigned long) rate;
  set_params();

  return samplerate;
}

double yunsdr_source_c::get_sample_rate( void )
{
  return samplerate;
}

osmosdr::freq_range_t yunsdr_source_c::get_freq_range( size_t chan )
{
  osmosdr::freq_range_t range;

  range += osmosdr::range_t( 70.0e6, 6000.0e6, 1.0 );

  return range;
}

double yunsdr_source_c::set_center_freq( double freq, size_t chan )
{
  frequency = (unsigned long long) freq;
  set_params();

  return freq;
}

double yunsdr_source_c::get_center_freq( size_t chan )
{
  return frequency;
}

double yunsdr_source_c::set_freq_corr( double ppm, size_t chan)
{
  _freq_corr = ppm;
  set_params();

  return ppm;
}

double yunsdr_source_c::get_freq_corr( size_t chan)
{
  return _freq_corr;
}

std::vector<std::string> yunsdr_source_c::get_gain_names( size_t chan )
{
  std::vector< std::string > gains;

  gains.push_back( "RF" );

  return gains;
}

osmosdr::gain_range_t yunsdr_source_c::get_gain_range( size_t chan)
{
  osmosdr::gain_range_t range;
  range += osmosdr::range_t( 1, 77, 1 );   // https://wiki.analog.com/resources/tools-software/linux-drivers/iio-transceiver/ad9361#rx_gain_control

  return range;
}

osmosdr::gain_range_t yunsdr_source_c::get_gain_range( const std::string & name,
                                                         size_t chan)
{
  osmosdr::gain_range_t range;

  range += osmosdr::range_t( 1, 77, 1 );

  return range;
}

bool yunsdr_source_c::set_gain_mode( bool automatic, size_t chan )
{
  gain_auto = automatic;
  set_params();

  return automatic;
}

bool yunsdr_source_c::get_gain_mode( size_t chan )
{
  return gain_auto;
}

double yunsdr_source_c::set_gain( double gain, size_t chan )
{
  gain_value = gain;
  set_params();

  return gain;
}

double yunsdr_source_c::set_gain( double gain, const std::string & name, size_t chan )
{
  gain_value = gain;
  set_params();

  return gain;
}

double yunsdr_source_c::get_gain( size_t chan )
{
  return gain_value;
}

double yunsdr_source_c::get_gain( const std::string & name, size_t chan )
{
  return gain_value;
}

std::vector< std::string > yunsdr_source_c::get_antennas( size_t chan )
{
  std::vector< std::string > antennas;

  antennas += get_antenna( chan );

  return antennas;
}

std::string yunsdr_source_c::set_antenna( const std::string & antenna, size_t chan )
{
  return get_antenna( chan );
}

std::string yunsdr_source_c::get_antenna( size_t chan )
{
  return "RX";
}

double yunsdr_source_c::set_bandwidth( double bw, size_t chan )
{
  if (bw == 0.0)
    bw = 0.8 * samplerate;  // auto bandwidth

  bandwidth = (unsigned long)bw;
  set_params();
  return bandwidth;
}

double yunsdr_source_c::get_bandwidth( size_t chan )
{
  return bandwidth;
}

void yunsdr_source_c::set_params( void )
{
  unsigned long long freq = ((double)frequency * (1.0 + _freq_corr * 0.000001));

  // FIXME: gain_mode string can be manual / slow_attack / fast_attack / hybrid
  _src->set_params( freq, samplerate, bandwidth,
                    gain_auto ? "fast_attack" : "manual", gain_value,
                    gain_auto ? "fast_attack" : "manual", gain_value,
                    "RX",
                    filter.c_str(), filter_auto );
}
