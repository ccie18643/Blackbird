/*
 * Copyright (c) 2001 - 2003
 *      Sebastian Majewski. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Sebastian Majewski
 *      and his contributors.
 * 4. Neither the name of Sebastian Majewski nor the names of his contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */


#include <netz.h>

#include "support.h"
#include "ph_packet.h"


c_string print_ospf_hello_packet(c_ospf_hello_packet);
c_string print_ospf_dd_packet(c_ospf_hello_packet);
c_string print_ospf_lsr_packet(c_ospf_hello_packet);
c_string print_ospf_lsu_packet(c_ospf_hello_packet);
c_string print_ospf_lsa_packet(c_ospf_hello_packet);

c_string print_ospf_lsa_header(c_ospf_lsa_header);


c_string ospf_packet_handler(c_packet_info packet_info)
{
    c_string output_string;

    c_ospf_header header(packet_info.packet);

    bool lls_present = false;
    bool md5_present = false;

    output_string.add("OSPF\t"); 

    output_string.add("TYPE %u (",
        header.get_type());

    switch(header.get_type())
    {
        case OSPF_TYPE_HELLO_PACKET:
            output_string.add("Hello");
            break;

        case OSPF_TYPE_DD_PACKET: 
            output_string.add("DD");
            break;

        case OSPF_TYPE_LSR_PACKET: 
            output_string.add("LSR");
            break;

        case OSPF_TYPE_LSU_PACKET: 
            output_string.add("LSU");
            break;

        case OSPF_TYPE_LSA_PACKET: 
            output_string.add("LSA");
            break;

        default: 
            output_string.add("Unknown");
    };

    output_string.add(")  ");


    string router_id_string[16];
    string area_id_string[16];
  
    conv_ip_str(router_id_string, header.get_routerid());
    conv_ip_str(area_id_string, header.get_areaid());

    output_string.add("ROUTER ID %s  AREA ID %s\n",
        router_id_string,
        area_id_string);


    output_string.add("\tVER %u  PLEN %u  (TPLEN %u)  CKSUM %u ",
        header.get_ver(),
        header.get_plen(),
	packet_info.packet_len,
        header.get_cksum());


    if(header.get_authtype() == OSPF_AUTHTYPE_CRYPTO)
    {
        output_string.add("(NONE)");
    }
    else
    {
        if(!cksum(packet_info.packet, header.get_plen()))
        {
            output_string.add("(OK)");
        }
        else
        {
            output_string.add("(BAD)");
        }
    }

    output_string.add("  AUTH TYPE %u (",
        header.get_authtype());

    switch(header.get_authtype())
    {
        case OSPF_AUTHTYPE_NULL:
            output_string.add("Null");
            break;
    
        case OSPF_AUTHTYPE_PASSWORD:
            output_string.add("Password");
            break;
 
        case OSPF_AUTHTYPE_CRYPTO:
	    md5_present = true;
            output_string.add("Crypto");
            break;

	default:
            output_string.add("Unknown");
    }

    output_string.add(")\n");

    if(header.get_authtype() == OSPF_AUTHTYPE_PASSWORD)
    {
        output_string.add("\tPASSWORD %s\n",
	    header.get_password());
    }



    if(header.get_authtype() == OSPF_AUTHTYPE_CRYPTO)
    {
        output_string.add("\tAUTH KEY ID %u  AUTH DATA LEN %u  AUTH SEQ %u\n",
            header.get_keyid(),
            header.get_adlen(),
            header.get_cryptoseq());
    }



    if(header.get_type() == OSPF_TYPE_HELLO_PACKET)
    {
        c_ospf_hello_packet hello_packet(packet_info.packet + OSPF_HEADER_LEN);

        string netmask_string[16];
 
        conv_ip_str(netmask_string, hello_packet.get_netmask());

        output_string.add("\tNETMASK %s  HELLO INT %u  ",
            netmask_string,
            hello_packet.get_hellointerval());


        output_string.add_hex("OPTIONS 0x%02X [ ",
            hello_packet.get_options());

 
        if(hello_packet.get_option_dc())
        {
            output_string.add("DC ");
        }

        if(hello_packet.get_option_l())
        {
            lls_present = true;
            output_string.add("L ");
        }

        if(hello_packet.get_option_np())
        {
            output_string.add("N/P ");
        }

        if(hello_packet.get_option_mc())
        {
            output_string.add("MC ");
        }

        if(hello_packet.get_option_e())
        {
            output_string.add("E ");
        }

        output_string.add("]");

        output_string.add("\n");


        string dr_string[16];
        string bdr_string[16];

        conv_ip_str(dr_string, hello_packet.get_dr());
        conv_ip_str(bdr_string, hello_packet.get_bdr());

        output_string.add("\tPRIORITY %u  DEAD INT %u  DR %s  BDR %s\n",
            hello_packet.get_priority(),
            hello_packet.get_deadinterval(),
            dr_string,
            bdr_string);


        u_int neighbors_len = header.get_plen() - OSPF_HEADER_LEN
            - OSPF_HELLO_PACKET_LEN ;

        for(u_int i = 0; i < neighbors_len / 4; i++)
        {
            string neighbor_string[16];
            conv_ip_str(neighbor_string, hello_packet.get_neighbor(i));

            output_string.add("\tNEIGHBOR %u - %s\n",
                i,
                neighbor_string);
        }

    }


    if(header.get_type() == OSPF_TYPE_DD_PACKET)  
    {
        c_ospf_dd_packet dd_packet(packet_info.packet + OSPF_HEADER_LEN); 

        output_string.add("\tMTU %u  ",
            dd_packet.get_mtu());

        output_string.add_hex("OPTIONS 0x%02X%02X [ ",
            dd_packet.get_options());

        if(dd_packet.get_option_i())   
        {
            output_string.add("I ");
        }

        if(dd_packet.get_option_m())
        {
            output_string.add("M ");
        }

        if(dd_packet.get_option_ms())
        {
            output_string.add("MS ");
        }

        output_string.add("]  SEQ %u\n",
            dd_packet.get_seq());

        u_int lsa_offset = OSPF_HEADER_LEN + OSPF_DD_PACKET_LEN;

        while (lsa_offset < header.get_plen())
        {
            c_ospf_lsa_header lsa_header(packet_info.packet + lsa_offset);

            output_string += print_ospf_lsa_header(lsa_header);
            output_string += "\n";

            lsa_offset += OSPF_LSA_HEADER_LEN;
        }
    }


    if(header.get_type() == OSPF_TYPE_LSR_PACKET)
    {
        u_int lsr_offset = OSPF_HEADER_LEN;
     
        while(lsr_offset < header.get_plen())
        { 
            c_ospf_lsr_packet lsr_packet(packet_info.packet + lsr_offset);

            output_string.add("\nLSA %u\tTYPE %u (",
                lsr_packet.get_type(),
                lsr_packet.get_type());
          
            switch(lsr_packet.get_type())
            {
                case OSPF_LSA_HEADER_TYPE_1:
                    output_string.add("Router");
                    break;

                case OSPF_LSA_HEADER_TYPE_2:
                    output_string.add("Network");
                    break;

                case OSPF_LSA_HEADER_TYPE_3:
                    output_string.add("Summary IP network");
                    break;

                case OSPF_LSA_HEADER_TYPE_4:
                    output_string.add("Summary ASBR");
                    break;

                case OSPF_LSA_HEADER_TYPE_5:
                    output_string.add("AS External");
                    break;

                case OSPF_LSA_HEADER_TYPE_7:
                    output_string.add("NSSA");
                    break;

                default:
                    output_string.add("Unknown");
            }

 
            output_string.add(")  LS ID %u  ",
                lsr_packet.get_id());

            string advrtr_string[16];

            conv_ip_str(advrtr_string, lsr_packet.get_advrtr());

            output_string.add("ADV RTR %s\n",
                advrtr_string);

            lsr_offset += OSPF_LSR_PACKET_LEN;
        }
    }


    if(header.get_type() == OSPF_TYPE_LSU_PACKET)
    {
        c_ospf_lsu_packet lsu_packet(packet_info.packet + OSPF_HEADER_LEN);

        output_string.add("\tLSA COUNT %u\n",
            lsu_packet.get_lcount());

        u_int lsa_offset = OSPF_HEADER_LEN + OSPF_LSU_PACKET_LEN;     

        u_int i = lsu_packet.get_lcount();

        while (i--)
        {
            c_ospf_lsa_header lsa_header(packet_info.packet + lsa_offset);

            if(lsa_header.get_option_l())
            {
                lls_present = true;
            }

            output_string += print_ospf_lsa_header(lsa_header);


            if(lsa_header.get_type() == OSPF_LSA_HEADER_TYPE_1)
            {
                c_ospf_lsa_1_body lsa_1_body(packet_info.packet + lsa_offset
                    + OSPF_LSA_HEADER_LEN);

                output_string.add_hex("  OPTIONS 0x%02X [ ",
                    lsa_1_body.get_flags()); 

                if(lsa_1_body.get_flag_v())
                {
                    output_string.add("V ");  
                }

                if(lsa_1_body.get_flag_e())
                {
                    output_string.add("E ");
                }

                if(lsa_1_body.get_flag_b())       
                {
                    output_string.add("B ");
                }

                if(lsa_1_body.get_flag_w())       
                {
                    output_string.add("W ");
                }

                if(lsa_1_body.get_flag_nt())       
                {
                    output_string.add("Nt ");
                }

                output_string.add("]  LINK COUNT %u\n",
                    lsa_1_body.get_lcount());

            
                u_int link_offset = lsa_offset + OSPF_LSA_HEADER_LEN
                    + OSPF_LSA_1_BODY_LEN;

                u_int j = lsa_1_body.get_lcount();

            

                while(j--)
                {
                    c_ospf_lsa_1_link lsa_1_link(packet_info.packet 
                        + link_offset);

                    output_string += "\t";

                    string id_string[16];
                    string data_string[16];

                    conv_ip_str(id_string, lsa_1_link.get_id());      
                    conv_ip_str(data_string, lsa_1_link.get_data());

                    output_string.add("ID %s  DATA %s  ",
                        id_string,
                        data_string);

                    output_string.add("TYPE %u (",
                        lsa_1_link.get_type());
                       
                    switch(lsa_1_link.get_type())      
                    {
                        case OSPF_LSA_1_LINK_TYPE_P2P:
                            output_string.add("P2P");
                            break;

                        case OSPF_LSA_1_LINK_TYPE_TRANSIT:
                            output_string.add("Transit");
                            break;

                        case OSPF_LSA_1_LINK_TYPE_STUB:
                            output_string.add("Stub");
                            break;

                        case OSPF_LSA_1_LINK_TYPE_VIRTUAL:
                            output_string.add("Virtual");
                            break;

                        default:
                            output_string.add("Unknown");
                    } 
 
                    output_string.add(")  METRIC %u",
                        lsa_1_link.get_metric());

                    output_string.add("\n");


                    u_int tos_offset = link_offset + OSPF_LSA_1_LINK_LEN;

                    u_int k = lsa_1_link.get_tcount();

                    while(k--)
                    {
                        /* we dont show TOS fields as they are obsolete */
                        tos_offset += OSPF_LSA_1_LINK_TOS_LEN;
                    }


                    link_offset += OSPF_LSA_1_LINK_LEN
                        + lsa_1_link.get_tcount() * OSPF_LSA_1_LINK_TOS_LEN;
                }
            }


            else if(lsa_header.get_type() == OSPF_LSA_HEADER_TYPE_2)
            {
                u_int link_offset = lsa_offset + OSPF_LSA_HEADER_LEN;

                output_string += "\n";

                while(link_offset < lsa_offset + lsa_header.get_len())
                {
                    c_ospf_lsa_2_link lsa_2_link(packet_info.packet
                        + link_offset);
  
                    output_string.add("\t");

                    string netmask_string[16];
                    string attrtr_string[16];

                    conv_ip_str(netmask_string, lsa_2_link.get_netmask());
                    conv_ip_str(attrtr_string, lsa_2_link.get_attrtr());

                    output_string.add("NETMASK %s  ATTACHED ROUTER %s\n",
                        netmask_string,
                        attrtr_string);

                    link_offset += OSPF_LSA_2_LINK_LEN;
                }
            }


            else if(lsa_header.get_type() == OSPF_LSA_HEADER_TYPE_3)
            {
                c_ospf_lsa_3_body lsa_3_link(packet_info.packet
                    + lsa_offset + OSPF_LSA_HEADER_LEN);

                string netmask_string[16];

                conv_ip_str(netmask_string, lsa_3_link.get_netmask());

                output_string.add("  NETMASK %s  METRIC %u\n",
                    netmask_string,
                    lsa_3_link.get_metric());

                /* we dont support TOS fields as they are obsolete */
            }


            else if(lsa_header.get_type() == OSPF_LSA_HEADER_TYPE_4)
            {
                c_ospf_lsa_4_body lsa_4_link(packet_info.packet    
                    + lsa_offset + OSPF_LSA_HEADER_LEN);    

                output_string.add("  METRIC %u\n",    
                    lsa_4_link.get_metric());

                /* we dont support TOS fields as they are obsolete */
            }

            else if(lsa_header.get_type() == OSPF_LSA_HEADER_TYPE_5)
            {
                c_ospf_lsa_5_body lsa_5_body(packet_info.packet    
                    + lsa_offset + OSPF_LSA_HEADER_LEN);    

                string netmask_string[16];

                conv_ip_str(netmask_string, lsa_5_body.get_netmask());  

                output_string.add("  NETMASK %s\n\tTYPE ",
                    netmask_string);

                if(lsa_5_body.get_type())
                {
                    output_string.add("E2");
                }
                else
                {
                    output_string.add("E1");
                }

                output_string.add("  METRIC %u  ",
                    lsa_5_body.get_metric());

                string fwdaddr_string[16];

                conv_ip_str(fwdaddr_string, lsa_5_body.get_fwdaddr());  

                output_string.add("FWD ADDR %s  TAG %u\n",
                    fwdaddr_string,
                    lsa_5_body.get_tag());
            }

            else if(lsa_header.get_type() == OSPF_LSA_HEADER_TYPE_7)
            {
                output_string.add("\n");
            }

            else
            {
                output_string.add("\n");
            }


           
            lsa_offset += lsa_header.get_len();
        }
    }


    if(header.get_type() == OSPF_TYPE_LSA_PACKET)
    {
        u_int lsa_offset = OSPF_HEADER_LEN;

        while (lsa_offset < header.get_plen())
        {
            c_ospf_lsa_header lsa_header(packet_info.packet + lsa_offset);

            output_string += print_ospf_lsa_header(lsa_header);
            output_string += "\n";

            lsa_offset += OSPF_LSA_HEADER_LEN;
        }

    }



    if(header.get_authtype() == OSPF_AUTHTYPE_CRYPTO)
    {
        output_string += print_hex_data(packet_info.packet
            + header.get_plen(), 16, true, "MD5\t");
    }


    if(lls_present)
    {
        c_ospf_lls lls(packet_info.packet + header.get_plen()
            + md5_present * 16);

        output_string.add("\n");
        output_string.add("LLS\tCKSUM %u ",
            lls.get_cksum());

        if(header.get_authtype() == OSPF_AUTHTYPE_CRYPTO)
        {
            output_string.add("(NONE)");
        }
        else if(!cksum(packet_info.packet + header.get_plen()
            + md5_present * 16, lls.get_dlen()))
        {
            output_string.add("(OK)");
        }
        else
        {
            output_string.add("(BAD)");
        }

        output_string.add("  DLEN %u",
            lls.get_dlen());

        output_string.add("\n");


        u_int i = 0;

        while(i < lls.get_dlen() - OSPF_LLS_LEN)
        {
            c_ospf_lls_tlv lls_tlv(packet_info.packet + header.get_plen() 
                + OSPF_LLS_LEN + md5_present * 16 + i);


            output_string.add("\nTLV\tTYPE %u ",
                lls_tlv.get_type());

            switch(lls_tlv.get_type())
            {
                case OSPF_LLS_TLV_TYPE_EXTOPT:
                {
                    c_ospf_lls_tlv_extopt lls_tlv_extopt(packet_info.packet
                        + header.get_plen()
                        + OSPF_LLS_LEN + md5_present * 16 + i);

                    output_string.add("(Extended Options)  VLEN %u\n",
                        lls_tlv.get_vlen());

                    output_string.add_hex("\tOPTIONS 0x%02X%02X%02X%02X [ ",
                        lls_tlv_extopt.get_flags());

                    if(lls_tlv_extopt.get_flag_rs())
                    {
                        output_string.add("RS ");
                    }

                    if(lls_tlv_extopt.get_flag_lr())
                    {
                        output_string.add("LR ");
                    }

                    output_string.add("]");

                    break;
                }

                case OSPF_LLS_TLV_TYPE_CAUTH:
                {
                    c_ospf_lls_tlv_cauth lls_tlv_cauth(packet_info.packet
                        + header.get_plen()
                        + OSPF_LLS_LEN + md5_present * 16 + i);

                    output_string.add("(Cryptographic Authentication)  "
                        "VLEN %u\n\tAUTH SEQ %u\n\t",
                        lls_tlv.get_vlen(),
                        lls_tlv_cauth.get_cryptoseq());

                   output_string += print_hex_data(packet_info.packet
                        + header.get_plen()
                        + OSPF_LLS_LEN + md5_present * 16 + i + 4, 16, false);


                    break;
                }

                default:
                    output_string += "(Unknown)";
            }

            output_string.add("\n");

            i += lls_tlv.get_vlen() + OSPF_LLS_TLV_LEN;
        }
    }
       
    output_string += debug(packet_info);

    output_string += print_line();

    return output_string;
}


c_string print_ospf_lsa_header(c_ospf_lsa_header lsa_header)
{
    c_string output_string;

    output_string.add("\nLSA %u\tAGE %u  ",
        lsa_header.get_type(),
        lsa_header.get_age());

    output_string.add_hex("OPTIONS 0x%02X [ ",
        lsa_header.get_options());

    if(lsa_header.get_option_dc())
    {
        output_string.add("DC ");
    }

    if(lsa_header.get_option_l())
    {
        output_string.add("L ");
    }

    if(lsa_header.get_option_np())
    {
        output_string.add("N/P ");
    }

    if(lsa_header.get_option_mc())
    {
        output_string.add("MC ");
    }

    if(lsa_header.get_option_e())
    {
         output_string.add("E ");
    }


    output_string.add("]  TYPE %u (",
        lsa_header.get_type());

    switch(lsa_header.get_type())
    {
        case OSPF_LSA_HEADER_TYPE_1:
            output_string.add("Router");
            break;

        case OSPF_LSA_HEADER_TYPE_2:
            output_string.add("Network");
            break;

        case OSPF_LSA_HEADER_TYPE_3:
            output_string.add("Summary IP network");
            break;

        case OSPF_LSA_HEADER_TYPE_4:
            output_string.add("Summary ASBR");
            break;

        case OSPF_LSA_HEADER_TYPE_5:
            output_string.add("AS External");
            break;

        case OSPF_LSA_HEADER_TYPE_7:
            output_string.add("NSSA");
            break;

        default:
            output_string.add("Unknown");
    }

    output_string.add(")\n");


    string id_string[16];
    string advrtr_string[16];

    conv_ip_str(id_string, lsa_header.get_id());
    conv_ip_str(advrtr_string, lsa_header.get_advrtr());

    output_string.add("\tLS ID %s  ADV RTR %s  SEQ %u\n",
        id_string,
        advrtr_string,
        lsa_header.get_seq());

    output_string.add("\tCKSUM %u  ",
        lsa_header.get_cksum());

    output_string.add("LEN %u",
        lsa_header.get_len());

    return output_string;
}

