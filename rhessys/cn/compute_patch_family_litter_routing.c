/*--------------------------------------------------------------*/
/*					                                            */
/*		compute_patch_family_routing		                    */
/*					                                            */
/*	compute_patch_family_routing.c - routes within patch family	*/
/*					                                            */
/*	NAME						                                */
/*	compute_patch_family_routing.c - routes within patch family	*/
/*					                                            */
/*	SYNOPSIS			                                        */
/*	void compute_patch_family_routing( 	                        */
/*						    struct zone_object *zone)           */
/*										                        */
/*	OPTIONS								                    	*/
/*										                        */
/*										                        */
/*	DESCRIPTION								                    */
/*  For all patch families in a zone, routes water between      */
/*	patches in each patch family. Routing is based on speudo-   */
/*	root access, and has distinct rules for root, unsaturated,  */
/*	and saturaetd zones, and transfers between patches are      */
/*  modified by sh_l and sh_g coefficients.                     */
/*                                                              */
/*	PROGRAMMER NOTES							                */
/*										                        */
/*	July, 2019 William Burke						            */
/*											                    */
/*--------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h> // i think not needed?
#include <math.h>
#include "rhessys.h"

void compute_patch_family_litter_routing(struct zone_object *zone,
                                  struct command_line_object *command_line)
{



    /*--------------------------------------------------------------*/
    /*	Local variable definition.			                        */
    /*--------------------------------------------------------------*/

    int pf;
    int i;
    int p_ct;            // number of patches in patch family
    int p_ct_skip;       // number of patches in patch family without skipped patches
    //double wet_mean;     // mean wetness for rz+unsat, meters water
    //double wet_mean_sat; // mean wetness for sat zone, meters water
    double area_sum;     // sum of areas in patch family
    double area_sum_g;   // sum of gaining patches area


    /*--------------------------------------------------------------*/
    /*	Loop through patch families in the zone   	                */
    /*--------------------------------------------------------------*/
    for (pf = 0; pf < zone[0].num_patch_families; pf++)
    {
        if (command_line[0].verbose_flag == -6)
            printf("\n--- Patch Family %d ---\n", zone[0].patch_families[pf][0].family_ID);

        /*--------------------------------------------------------------*/
        /*	Patch family definitions & initializations                  */
        /*--------------------------------------------------------------*/
        p_ct = zone[0].patch_families[pf][0].num_patches_in_fam; // for simplicity line46

        /* Definitions */
        int skip1[p_ct], skip2[p_ct], skip3[p_ct], skip4[p_ct];         // 0 = skip, 1 = lose, 2 = gain
        int no_veg_patch[p_ct];
        double dL_c1[p_ct], dL_c2[p_ct], dL_c3[p_ct], dL_c4[p_ct];
        double dL_n1[p_ct], dL_n2[p_ct], dL_n3[p_ct], dL_n4[p_ct];
        double dG_c1[p_ct], dG_c2[p_ct], dG_c3[p_ct], dG_c4[p_ct];
        double dG_n1[p_ct], dG_n2[p_ct], dG_n3[p_ct], dG_n4[p_ct];

        int p_ct_skip;
        int p_no_veg;
        int area_sum;
        int area_sum_g; //area sum of all gain (no-veg) patches
        int area_sum_l1, area_sum_l2, area_sum_l3, area_sum_l4; // area sum of loss patches
        int area_sum_g_act1, area_sum_g_act2, area_sum_g_act3, area_sum_g_act4; //area sum of actual gain patches
        double dL_c_act1, dL_c_act2, dL_c_act3, dL_c_act4;
        //double dL_c_pot;
        double dL_n_act1, dL_n_act2, dL_n_act3, dL_n_act4;
        //double dL_n_pot;
        double dG_c_act1, dG_c_act2, dG_c_act3, dG_c_act4; // gaining actual carbon
        double dG_c_pot1, dG_c_pot2, dG_c_pot3, dG_c_pot4;
        double dG_n_act1, dG_n_act2, dG_n_act3, dG_n_act4;
        double dG_n_pot1, dG_n_pot2, dG_n_pot3, dG_n_pot4;

        //double litter_c_transfer;
        //double litter_n_transfer;
        double litter_c_adjust1, litter_c_adjust2, litter_c_adjust3, litter_c_adjust4;
        double litter_n_adjust1, litter_n_adjust2, litter_n_adjust3, litter_n_adjust4;
        double litr1c_mean, litr2c_mean, litr3c_mean, litr4c_mean;
        double litr1c_mean_after, litr2c_mean_after, litr3c_mean_after, litr4c_mean_after; // for mean before routing
        double litr1n_mean, litr2n_mean, litr3n_mean, litr4n_mean;
        double litr1n_mean_after, litr2n_mean_after, litr3n_mean_after, litr4n_mean_after; //for mean before routing
        double patch_area;
        int loss_patch1, loss_patch2, loss_patch3, loss_patch4;
        double litter_c_adjust_total1, litter_c_adjust_total2, litter_c_adjust_total3, litter_c_adjust_total4;
        double litter_n_adjust_total1, litter_n_adjust_total2, litter_n_adjust_total3, litter_n_adjust_total4;
        double rooting_depth_mean;

        struct  patch_object            *patches;
        /* Initializations */
        skip1[p_ct], skip2[p_ct], skip3[p_ct], skip4[p_ct];     // 0 = skip, 1 = lose, 2 = gain
        no_veg_patch[p_ct];
        dL_c1[p_ct], dL_c2[p_ct], dL_c3[p_ct], dL_c4[p_ct];// litter carbob loss
        dL_n1[p_ct], dL_n2[p_ct], dL_n3[p_ct], dL_n4[p_ct];
        dG_c1[p_ct], dG_c2[p_ct], dG_c3[p_ct], dG_c4[p_ct];
        dG_n1[p_ct], dG_n2[p_ct], dG_n3[p_ct], dG_n4[p_ct];

        p_ct_skip = 0; //to do?
        p_no_veg = 0;
        area_sum = 0;
        area_sum_g = 0;
        area_sum_g_act1 = 0, area_sum_g_act2 = 0, area_sum_g_act3 = 0, area_sum_g_act4 = 0;
        area_sum_l1 = 0, area_sum_l2 = 0, area_sum_l3 = 0, area_sum_l4 = 0;
        dL_c_act1 = 0, dL_c_act2 = 0, dL_c_act3 = 0, dL_c_act4 = 0;
       // dL_c_pot = 0;
        dL_n_act1 = 0, dL_n_act2 = 0, dL_n_act3 = 0, dL_n_act4 = 0;
       // dL_n_pot = 0;
        dG_c_act1 = 0, dG_c_act2 = 0, dG_c_act3 = 0, dG_c_act4 = 0;
        dG_n_act1 = 0, dG_n_act2 = 0, dG_n_act3 = 0, dG_n_act4 = 0;
        dG_c_pot1 = 0, dG_c_pot2 = 0, dG_c_pot3 = 0, dG_c_pot4 = 0;
        dG_n_pot1 = 0, dG_n_pot2 = 0, dG_n_pot3 = 0, dG_n_pot4 = 0;


        //litter_c_transfer = 0;
        //litter_n_transfer = 0;
        litter_c_adjust1 = 0, litter_c_adjust2 = 0, litter_c_adjust3 =0, litter_c_adjust4 = 0;
        litter_n_adjust1 = 0, litter_n_adjust2 = 0, litter_n_adjust3 =0, litter_n_adjust4 = 0;
        litr1c_mean = 0, litr2c_mean = 0, litr3c_mean = 0, litr4c_mean = 0;
        litr1c_mean_after = 0, litr2c_mean_after = 0, litr3c_mean_after = 0, litr4c_mean_after = 0; // for liter before routing
        litr1n_mean = 0, litr2n_mean = 0, litr3n_mean = 0, litr4n_mean = 0;
        litr1n_mean_after = 0, litr2n_mean_after = 0, litr3n_mean_after = 0, litr4n_mean_after = 0; // for litter before routing

        patch_area = 0;
        loss_patch1 = 0, loss_patch2 = 0, loss_patch3 = 0, loss_patch4 = 0;
        litter_c_adjust_total1 = 0.0, litter_c_adjust_total2= 0, litter_c_adjust_total3 =0, litter_c_adjust_total4 = 0;
        litter_n_adjust_total1 = 0.0, litter_n_adjust_total2 = 0, litter_n_adjust_total3 =0, litter_n_adjust_total4 = 0;
        rooting_depth_mean = 0.0;


        /*--------------------------------------------------------------*/
        /*	Loop 1 - Get litter for each patch family                */
        /*--------------------------------------------------------------*/
        if (command_line[0].verbose_flag == -6)
            printf("||Loop 1 Pre-litter-transfer ||\n");



        for (i = 0; i < zone[0].patch_families[pf][0].num_patches_in_fam; i++)
        {
            /* Initializations */
           // litter_c_transfer = 0; // do i need 1, 2, 3,4 pool
            // litter_n_transfer = 0;
            patches = zone[0].patch_families[pf][0].patches[i];

            // if both sh coefficients are not 0, include patch
            if (zone[0].patch_families[pf][0].num_patches_in_fam > 1)
            {
                // if sharing coefs > 0, include this patch in subsiquent analyses
                skip1[i] = 1;
                skip2[i] = 1;
                skip3[i] = 1;
                skip4[i] = 1;

                if (command_line[0].verbose_flag == -6)
                {
                    printf("\n||ID %d | Area %f \n | litr1c %f | litr1n %f \n | litr2c %f | litr2n %f \n | litr3c %f | litr3n %f \n | litr4c %f | litr4n %f \n",
                           patches[0].ID,
                           patches[0].area,
                           patches[0].litter_cs.litr1c,
                           patches[0].litter_ns.litr1n,
                           patches[0].litter_cs.litr2c,
                           patches[0].litter_ns.litr2n,
                           patches[0].litter_cs.litr3c,
                           patches[0].litter_ns.litr3n,
                           patches[0].litter_cs.litr4c,
                           patches[0].litter_ns.litr4n
                           );
                }
                // incrament mean wetness based on storage (rz+unsat or sat) * area
                litr1c_mean += patches[0].litter_cs.litr1c * patches[0].area;
                litr1n_mean += patches[0].litter_ns.litr1n * patches[0].area;
                // litter2c need initialize todo
                litr2c_mean += patches[0].litter_cs.litr2c * patches[0].area;
                litr2n_mean += patches[0].litter_ns.litr2n * patches[0].area;
                // litter3c todo
                litr3c_mean += patches[0].litter_cs.litr3c * patches[0].area;
                litr3n_mean += patches[0].litter_ns.litr3n * patches[0].area;
                // litter4c todo
                litr4c_mean += patches[0].litter_cs.litr4c * patches[0].area;
                litr4n_mean += patches[0].litter_ns.litr4n * patches[0].area;

                //for mineralization if no-veg patches use mean_rootdepth instead of zero
                rooting_depth_mean += patches[0].rootzone.depth * patches[0].area;
                // area sum (patch fam without skipped patches)
                area_sum += patches[0].area;

                // patch count (all patches)
                p_ct_skip += 1;
                // count no veg patches
                if (patches[0].canopy_strata[0][0].defaults[0][0].epc.veg_type == NON_VEG){


                        // do I need to save the ID which one is no veg patches? create a vevtor save i?
                        no_veg_patch[p_no_veg] = patches[0].ID;
                        p_no_veg +=1; //count how many no veg patches
                        // incrament gainer area
                        area_sum_g += patches[0].area;
                         if (command_line[0].verbose_flag == -6) {printf("\n No veg patches are %d | ", patches[0].ID);}
                    }

            }
            else
            {
                // sharing coefs are 0, skip this patch in this and subsiquent routing loops
                skip1[i] = 0;
                skip2[i] = 0;
                skip3[i] = 0;
                skip4[i] = 0;

            }
        } // end loop 1

        // Get mean wetness - vol water/(total patch family) area - units are meters depth
        if (area_sum > ZERO)
        {
            litr1c_mean /= area_sum, litr2c_mean /= area_sum, litr3c_mean /= area_sum, litr4c_mean /= area_sum;
            litr1n_mean /= area_sum, litr2n_mean /= area_sum, litr3n_mean /= area_sum, litr4n_mean /= area_sum;
            rooting_depth_mean /= area_sum;
        }
        else
        {
            litr1c_mean = 0.0, litr2c_mean = 0.0, litr3c_mean = 0.0, litr4c_mean = 0.0;
            litr1n_mean = 0.0, litr2n_mean = 0.0, litr3n_mean = 0.0, litr4n_mean = 0.0;
            rooting_depth_mean = 0.0;
        }

        if (command_line[0].verbose_flag == -6){
            printf("\n**|| Before redistribution Mean litter1c = %f|| Mean litter1n = %f ", litr1c_mean, litr1n_mean);
            printf("\n**|| Before redistribution Mean litter2c = %f|| Mean litter2n = %f ", litr2c_mean, litr2n_mean);
            printf("\n**|| Before redistribution Mean litter3c = %f|| Mean litter3n = %f ", litr3c_mean, litr3n_mean);
            printf("\n**|| Before redistribution Mean litter4c = %f|| Mean litter4n = %f ", litr4c_mean, litr4n_mean);
            printf("|| Mean root depth is %f \n", rooting_depth_mean);}

        /*--------------------------------------------------------------*/
        /*  loop 2, loop through losing (>mean) patches                 */
        /*--------------------------------------------------------------*/
        if (command_line[0].verbose_flag == -6){
            printf("\n ==============================\n");
            printf("|| Loop 2 Losing litter (>mean) Patches ||\n");
            }

        for (i = 0; i < zone[0].patch_families[pf][0].num_patches_in_fam; i++)
        {

            patches = zone[0].patch_families[pf][0].patches[i];

            if (command_line[0].verbose_flag == -6)
                printf("\n ID %d |", patches[0].ID);
            /* litter 1 pool loop */
            // if - no skip, patches are veg patches, and have litter >zer0
            if (skip1[i] > 0 && (patches[0].litter_cs.litr1c) > ZERO &&
                 (patches[0].canopy_strata[0][0].defaults[0][0].epc.veg_type) != NON_VEG &&
                 (p_ct_skip > p_no_veg) && patches[0].litter_cs.litr1c > litr1c_mean &&
                  patches[0].litter_ns.litr1n > litr1n_mean)
            {
                loss_patch1 += 1;  //todo
                patch_area = patches[0].area;

                dL_c1[i] = (patches[0].litter_cs.litr1c - litr1c_mean) * patch_area;// make sure p_ct_skip > p_no_veg
                         //* zone[0].defaults[0][0].sh_litter;//only veg patch lose litter
                dL_c_act1 += dL_c1[i];// this is important to track the total litter

                // litter 1 n
                dL_n1[i] = (patches[0].litter_ns.litr1n - litr1n_mean) * patch_area;// make sure p_ct_skip > p_no_veg
                         //* zone[0].defaults[0][0].sh_litter;//only veg patch lose litter
                dL_n_act1 += dL_n1[i]; // this is important to track the total litter


                if (command_line[0].verbose_flag == -6)
                    printf("\n||[losing litter1 mean]%f ", dL_c1[i] / patch_area);
                if (command_line[0].verbose_flag == -6)
                    printf("\n||[losing litter1 sum]%f || [total cumulative losing is sum %f]", dL_c1[i], dL_c_act1);

                //remove litter from these patches
                // make them to be mean
                patches[0].litter_cs.litr1c = litr1c_mean;
                patches[0].litter_ns.litr1n = litr1n_mean;
                area_sum_l1 += patches[0].area;

            }

            else if (skip1[i] > 0 && (patches[0].canopy_strata[0][0].defaults[0][0].epc.veg_type) == NON_VEG)
            {
                // is a gaining patch
                skip1[i] = 2; // do I need 1 ,2,3, 4 for skip
                dL_c1[i] = 0; // 1, 2, 3, 4
                dL_n1[i] = 0;
                // here set no veg patch a mean root depth for mineralization
                patches[0].rooting_depth_mean = rooting_depth_mean;
            }
            else
            {
                dL_c1[i] = 0;
                dL_n1[i] = 0;
            } // end of litter1

        /* litter 2 */
            if (skip2[i] > 0 && (patches[0].litter_cs.litr2c) > ZERO &&
                 (patches[0].canopy_strata[0][0].defaults[0][0].epc.veg_type) != NON_VEG &&
                 (p_ct_skip > p_no_veg) && patches[0].litter_cs.litr2c > litr2c_mean &&  // p_ct_skip number of total patches > p_no_veg number of no veg patches, mean not all are no-veg
                  patches[0].litter_ns.litr2n > litr2n_mean)
            {
                loss_patch2 += 1; //todo
                patch_area = patches[0].area;

                dL_c2[i] = (patches[0].litter_cs.litr2c - litr2c_mean) * patch_area;// make sure p_ct_skip > p_no_veg
                         //* zone[0].defaults[0][0].sh_litter;//only veg patch lose litter
                dL_c_act2 += dL_c2[i];// this is important to track the total litter

                // litter 1 n
                dL_n2[i] = (patches[0].litter_ns.litr2n - litr2n_mean) * patch_area;// make sure p_ct_skip > p_no_veg
                         //* zone[0].defaults[0][0].sh_litter;//only veg patch lose litter
                dL_n_act2 += dL_n2[i]; // this is important to track the total litter


                if (command_line[0].verbose_flag == -6)
                    printf("\n||[losing litter2 mean]%f ", dL_c2[i] / patch_area);
                if (command_line[0].verbose_flag == -6)
                    printf("\n||[losing litter2 sum]%f || [total cumulative losing is sum %f]", dL_c2[i], dL_c_act2);

                //remove litter from these patches
                // make them to be mean
                patches[0].litter_cs.litr2c = litr2c_mean;
                patches[0].litter_ns.litr2n = litr2n_mean;
                area_sum_l2 += patches[0].area;

            }

            else if (skip2[i] > 0 && (patches[0].canopy_strata[0][0].defaults[0][0].epc.veg_type) == NON_VEG)
            {
                // is a gaining patch
                skip2[i] = 2; // to do skip = 1 is losing patches, skip =2 is gaining patch skip =0 skip patch
                dL_c2[i] = 0; // 1, 2, 3, 4
                dL_n2[i] = 0;
                // here set no veg patch a mean root depth for mineralization
                //patches[0].rooting_depth_mean = rooting_depth_mean;
            }
            else
            {
                dL_c2[i] = 0;
                dL_n2[i] = 0;
            } // end of litter2


        /* litter 3 */
            if (skip3[i] > 0 && (patches[0].litter_cs.litr3c) > ZERO &&
                 (patches[0].canopy_strata[0][0].defaults[0][0].epc.veg_type) != NON_VEG &&
                 (p_ct_skip > p_no_veg) && patches[0].litter_cs.litr3c > litr3c_mean &&  // p_ct_skip number of total patches > p_no_veg number of no veg patches, mean not all are no-veg
                  patches[0].litter_ns.litr3n > litr3n_mean)
            {
                loss_patch3 += 1; //todo
                patch_area = patches[0].area;

                dL_c3[i] = (patches[0].litter_cs.litr3c - litr3c_mean) * patch_area;// make sure p_ct_skip > p_no_veg
                         //* zone[0].defaults[0][0].sh_litter;//only veg patch lose litter
                dL_c_act3 += dL_c3[i];// this is important to track the total litter

                // litter 1 n
                dL_n3[i] = (patches[0].litter_ns.litr3n - litr3n_mean) * patch_area;// make sure p_ct_skip > p_no_veg
                         //* zone[0].defaults[0][0].sh_litter;//only veg patch lose litter
                dL_n_act3 += dL_n3[i]; // this is important to track the total litter


                if (command_line[0].verbose_flag == -6)
                    printf("\n||[losing litter3 mean]%f ", dL_c3[i] / patch_area);
                if (command_line[0].verbose_flag == -6)
                    printf("\n||[losing litter3 sum]%f || [total cumulative losing is sum %f]", dL_c3[i], dL_c_act3);

                //remove litter from these patches
                // make them to be mean
                patches[0].litter_cs.litr3c = litr3c_mean;
                patches[0].litter_ns.litr3n = litr3n_mean;
                area_sum_l3 += patches[0].area;

            }

            else if (skip3[i] > 0 && (patches[0].canopy_strata[0][0].defaults[0][0].epc.veg_type) == NON_VEG)
            {
                // is a gaining patch
                skip3[i] = 2; // to do
                dL_c3[i] = 0; // 1, 2, 3, 4
                dL_n3[i] = 0;
                // here set no veg patch a mean root depth for mineralization
                //patches[0].rooting_depth_mean = rooting_depth_mean;
            }
            else
            {
                dL_c3[i] = 0;
                dL_n3[i] = 0;
            } // end of litter3

        /* litter 4 */
            if (skip4[i] > 0 && (patches[0].litter_cs.litr4c) > ZERO &&
                 (patches[0].canopy_strata[0][0].defaults[0][0].epc.veg_type) != NON_VEG &&
                 (p_ct_skip > p_no_veg) && patches[0].litter_cs.litr4c > litr4c_mean &&  // p_ct_skip number of total patches > p_no_veg number of no veg patches, mean not all are no-veg
                  patches[0].litter_ns.litr4n > litr4n_mean)
            {
                loss_patch4 += 1; //todo
                patch_area = patches[0].area;

                dL_c4[i] = (patches[0].litter_cs.litr4c - litr4c_mean) * patch_area;// make sure p_ct_skip > p_no_veg
                         //* zone[0].defaults[0][0].sh_litter;//only veg patch lose litter
                dL_c_act4 += dL_c4[i];// this is important to track the total litter

                // litter 4n
                dL_n4[i] = (patches[0].litter_ns.litr4n - litr4n_mean) * patch_area;// make sure p_ct_skip > p_no_veg
                         //* zone[0].defaults[0][0].sh_litter;//only veg patch lose litter
                dL_n_act4 += dL_n4[i]; // this is important to track the total litter


                if (command_line[0].verbose_flag == -6)
                    printf("\n||[losing litter4 mean]%f ", dL_c4[i] / patch_area);
                if (command_line[0].verbose_flag == -6)
                    printf("\n||[losing litter4 sum]%f || [total cumulative losing is sum %f]", dL_c4[i], dL_c_act4);

                //remove litter from these patches
                // make them to be mean
                patches[0].litter_cs.litr4c = litr4c_mean;
                patches[0].litter_ns.litr4n = litr4n_mean;
                area_sum_l4 += patches[0].area;

            }

            else if (skip4[i] > 0 && (patches[0].canopy_strata[0][0].defaults[0][0].epc.veg_type) == NON_VEG)
            {
                // is a gaining patch
                skip4[i] = 2; // to do
                dL_c4[i] = 0; // 1, 2, 3, 4
                dL_n4[i] = 0;
                // here set no veg patch a mean root depth for mineralization
                //patches[0].rooting_depth_mean = rooting_depth_mean;
            }
            else
            {
                dL_c4[i] = 0;
                dL_n4[i] = 0;
            } // end of litter4
        if (command_line[0].verbose_flag == -6){
            printf("\n ||Litter 1C Loses: [act]%f, Litter 1N loses: [act]%f", dL_c_act1, dL_n_act1);
            printf("\n ||Litter 2C Loses: [act]%f, Litter 2N loses: [act]%f", dL_c_act2, dL_n_act2);
            printf("\n ||Litter 3C Loses: [act]%f, Litter 3N loses: [act]%f", dL_c_act3, dL_n_act3);
            printf("\n ||Litter 4C Loses: [act]%f, Litter 4N loses: [act]%f", dL_c_act4, dL_n_act4);
            }


        } // end loop 2

        /*--------------------------------------------------------------*/
        /*  loop 3, loop through gaining (<mean) patches              	*/
        /*--------------------------------------------------------------*/

        if (command_line[0].verbose_flag == -6){
            printf("\n ==============================\n");
            printf("\n|| Loop 3 Litter Gaining (No-veg) Patches || ");
        }


        for (i = 0; i < zone[0].patch_families[pf][0].num_patches_in_fam; i++)
        {

            patches = zone[0].patch_families[pf][0].patches[i];
            patch_area = patches[0].area;
            // route the litter to no-veg patches(gainers)
            // litter 1 pool
            if (skip1[i] == 2 &&
                (patches[0].canopy_strata[0][0].defaults[0][0].epc.veg_type) == NON_VEG &&
                 patches[0].litter_cs.litr1c < litr1c_mean &&
                 patches[0].litter_ns.litr1n < litr1n_mean)
            {
                if (command_line[0].verbose_flag == -6)
                    printf("\n || Litter1 gaining ID %d", patches[0].ID);
                // the gain is equally distributed among no-veg patches
                // what if one patch has litter > mean, so the number of gain patches is different from # of no-veg patches
                //TODO: should add check to make sure sh_litter >0 and <1
                dG_c1[i] = dL_c_act1 /area_sum_g * patch_area * zone[0].defaults[0][0].sh_litter;// this the mean for each patch todo
                dG_c_act1 += dG_c1[i];
                dG_c_pot1 +=  dL_c_act1 /area_sum_g * patch_area; // here is only one sum_g since it is the sum of all no-veg patches

                dG_n1[i] = dL_n_act1/area_sum_g * patch_area  * zone[0].defaults[0][0].sh_litter;
                dG_n_act1 += dG_n1[i];
                dG_n_pot1 += dL_n_act1 /area_sum_g * patch_area ;


                if (command_line[0].verbose_flag == -6){
                    printf("\n||[actual litter1 Gain]%f \n", dG_c1[i]);
                    printf("|| [cumulative actual litter1 Gain]%f \n || [cumulative potential litter1 Gain]%f \n", dG_c_act1, dG_c_pot1);
                    printf("|| [before add litter] the litr1c is %f \n", patches[0].litter_cs.litr1c);}

                // litter carbon gain
                patches[0].litter_cs.litr1c += (dG_c1[i] / patch_area);
                patches[0].litter_ns.litr1n += (dG_n1[i] / patch_area);


                if (command_line[0].verbose_flag == -6){
                    printf("\n||[litter 1c gain]%f ", (dG_c1[i] / patch_area));
                    printf("\n||[after add litter], [the litr1c is] %f, [the amount of litter is transfered] %f\n", patches[0].litter_cs.litr1c, (dG_c1[i] / patch_area));
                    }


                                   //if the act gain is smaller than potential gain due to parameters, adjust it too ??
                  if (dG_c_pot1 > dG_c_act1 && dG_n_pot1 > dG_n_act1)  {
                   if (command_line[0].verbose_flag == -6)
                   printf("\n loop 4 adjust differences between actual gain and potential gain for litter1 [act]%f, [pot]%f \n", dG_c_act1, dG_c_pot1);//improve
                   litter_c_adjust_total1 += (dG_c_pot1 - dG_c_act1);
                   litter_n_adjust_total1 += (dG_n_pot1 - dG_n_act1);

                  }

                 // here needs to check if the gaining patches have litter more than mean, then adjust it

                 if (patches[0].litter_cs.litr1c > litr1c_mean && //? should be &&
                     patches[0].litter_ns.litr1n > litr1n_mean ){


                     litter_c_adjust_total1 += (patches[0].litter_cs.litr1c - litr1c_mean)*patch_area; //to do
                     litter_n_adjust_total1 += (patches[0].litter_ns.litr1n - litr1n_mean)*patch_area;
                     // make the no-veg gain patches litter mean
                     patches[0].litter_cs.litr1c = litr1c_mean;
                     patches[0].litter_ns.litr1n = litr1n_mean;

                    }
                if (command_line[0].verbose_flag == -6)
                    printf("\n || Litter1 adjusting [1c]%f, [1n]%f \n", litter_c_adjust_total1, litter_n_adjust_total1);

                // incrament gainer area
                area_sum_g_act1 += patches[0].area; //to do
            }// end skip==2 and no veg
            else
            {
                dG_c1[i] = 0;
                dG_n1[i] = 0;
            }
                //improve what is area_sum_g_act4 < area_sum_g and dG_c_act4 >ZERO; then adjust too
         if (area_sum_g_act1 < area_sum_g && dG_c_act1 >ZERO){
            if (command_line[0].verbose_flag == -6)
             printf("\n [Adjust] actual gaining no-veg patches are not all no-veg patches for litter 1");
             litter_c_adjust_total1 += dL_c_act1 /area_sum_g *(area_sum_g - area_sum_g_act1);
             litter_n_adjust_total1 += dL_n_act1 /area_sum_g *(area_sum_g - area_sum_g_act1);
            }

        /* litter 2 */
            if (skip2[i] == 2 &&
                (patches[0].canopy_strata[0][0].defaults[0][0].epc.veg_type) == NON_VEG &&
                 patches[0].litter_cs.litr2c < litr2c_mean &&
                 patches[0].litter_ns.litr2n < litr2n_mean)
            {
                if (command_line[0].verbose_flag == -6)
                    printf("\n || Litter2 gaining ID %d", patches[0].ID);
                // the gain is equally distributed among no-veg patches
                // what if one patch has litter > mean, so the number of gain patches is different from # of no-veg patches

                dG_c2[i] = dL_c_act2 /area_sum_g * patch_area * zone[0].defaults[0][0].sh_litter;// this the mean for each patch todo
                dG_c_act2 += dG_c2[i];
                dG_c_pot2 +=  dL_c_act2 /area_sum_g * patch_area; // here is only one sum_g since it is the sum of all no-veg patches

                dG_n2[i] = dL_n_act2/area_sum_g * patch_area  * zone[0].defaults[0][0].sh_litter;
                dG_n_act2 += dG_n2[i];
                dG_n_pot2 += dL_n_act2 /area_sum_g * patch_area ;


                if (command_line[0].verbose_flag == -6){
                    printf("\n||[actual litter2 Gain]%f \n", dG_c2[i]);
                    printf("|| [cumulative actual litter2 Gain]%f \n || [cumulative potential litter2 Gain]%f \n", dG_c_act2, dG_c_pot2);
                    printf("|| [before add litter] the litr2c is %f \n", patches[0].litter_cs.litr2c);}

                // litter carbon gain
                patches[0].litter_cs.litr2c += (dG_c2[i] / patch_area);
                patches[0].litter_ns.litr2n += (dG_n2[i] / patch_area);


                if (command_line[0].verbose_flag == -6){
                    printf("\n||[litter 2c gain]%f ", (dG_c2[i] / patch_area));
                    printf("\n||[after add litter2], [the litr2c is] %f, [the amount of litter is transfered] %f\n", patches[0].litter_cs.litr2c, (dG_c2[i] / patch_area));}

                                  //if the act gain is smaller than potential gain due to parameters, adjust it too ??
                if (dG_c_pot2 > dG_c_act2 && dG_n_pot2 > dG_n_act2)  {
                  if (command_line[0].verbose_flag == -6)
                  printf("\n loop 4 adjust differences between actual gain and potential gain for litter2 [act]%f, [pot]%f \n", dG_c_act2, dG_c_pot2);//improve
                  litter_c_adjust_total2 += (dG_c_pot2 - dG_c_act2);
                  litter_n_adjust_total2 += (dG_n_pot2 - dG_n_act2);

                  }
                 // here needs to check if the gaining patches have litter more than mean, then adjust it

                 if (patches[0].litter_cs.litr2c > litr2c_mean &&
                     patches[0].litter_ns.litr2n > litr2n_mean ){


                     litter_c_adjust_total2 += (patches[0].litter_cs.litr2c - litr2c_mean)*patch_area; //to do
                     litter_n_adjust_total2 += (patches[0].litter_ns.litr2n - litr2n_mean)*patch_area;
                     // make the no-veg gain patches litter mean
                     patches[0].litter_cs.litr2c = litr2c_mean;
                     patches[0].litter_ns.litr2n = litr2n_mean;

                    }



                // incrament gainer area
                area_sum_g_act2 += patches[0].area; //to do
            }// end skip==2 and no veg
            else
            {
                dG_c2[i] = 0;
                dG_n2[i] = 0;
            }
                //improve what is area_sum_g_act4 < area_sum_g and dG_c_act4 >ZERO; then adjust too
         if (area_sum_g_act2 < area_sum_g && dG_c_act2 >ZERO){
             if (command_line[0].verbose_flag == -6)
             printf("/n [Adjust] actual gaining no-veg patches are not all no-veg patches for litter 2");
             litter_c_adjust_total2 += dL_c_act2 /area_sum_g *(area_sum_g - area_sum_g_act2);
             litter_n_adjust_total2 += dL_n_act2 /area_sum_g *(area_sum_g - area_sum_g_act2);
            }

        /* litter 3 */
            if (skip3[i] == 2 &&
                (patches[0].canopy_strata[0][0].defaults[0][0].epc.veg_type) == NON_VEG &&
                 patches[0].litter_cs.litr3c < litr3c_mean &&
                 patches[0].litter_ns.litr3n < litr3n_mean)
            {
                if (command_line[0].verbose_flag == -6)
                    printf("\n || Litter3 gaining ID %d", patches[0].ID);
                // the gain is equally distributed among no-veg patches
                // what if one patch has litter > mean, so the number of gain patches is different from # of no-veg patches

                dG_c3[i] = dL_c_act3 /area_sum_g * patch_area * zone[0].defaults[0][0].sh_litter;// this the mean for each patch todo
                dG_c_act3 += dG_c3[i];
                dG_c_pot3 +=  dL_c_act3 /area_sum_g * patch_area; // here is only one sum_g since it is the sum of all no-veg patches

                dG_n3[i] = dL_n_act3/area_sum_g * patch_area  * zone[0].defaults[0][0].sh_litter;
                dG_n_act3 += dG_n3[i];
                dG_n_pot3 += dL_n_act3 /area_sum_g * patch_area ;


                if (command_line[0].verbose_flag == -6){
                    printf("\n||[actual litter3 Gain]%f \n", dG_c3[i]);
                    printf("|| [cumulative actual litter3 Gain]%f \n || [cumulative potential litter3 Gain]%f \n", dG_c_act3, dG_c_pot3);
                    printf("|| [before add litter] the litr3c is %f \n", patches[0].litter_cs.litr3c);}

                // litter carbon gain
                patches[0].litter_cs.litr3c += (dG_c3[i] / patch_area);
                patches[0].litter_ns.litr3n += (dG_n3[i] / patch_area);


                if (command_line[0].verbose_flag == -6){
                    printf("\n||[litter 3c gain]%f ", (dG_c3[i] / patch_area));
                    printf("\n||[after add litter], [the litr3c is] %f, [the amount of litter is transfered] %f\n", patches[0].litter_cs.litr3c, (dG_c3[i] / patch_area));}

                 // here needs to check if the gaining patches have litter more than mean, then adjust it

                   //if the act gain is smaller than potential gain due to parameters, adjust it too ??
                if (dG_c_pot3 > dG_c_act3 && dG_n_pot3 > dG_n_act3)  {
                  if (command_line[0].verbose_flag == -6)
                  printf("\n loop 4 adjust differences between actual gain and potential gain for litter3 [act]%f, [pot]%f \n", dG_c_act3, dG_c_pot3);//improve                    //printf("/n adjust differences between actual gain and potential gain for litter3");//improve
                  litter_c_adjust_total3 += (dG_c_pot3 - dG_c_act3);
                  litter_n_adjust_total3 += (dG_n_pot3 - dG_n_act3);

                  }
                 if (patches[0].litter_cs.litr3c > litr3c_mean &&
                     patches[0].litter_ns.litr3n > litr3n_mean ){


                     litter_c_adjust_total3 += (patches[0].litter_cs.litr3c - litr3c_mean)*patch_area; //to do
                     litter_n_adjust_total3 += (patches[0].litter_ns.litr3n - litr3n_mean)*patch_area;
                     // make the no-veg gain patches litter mean
                     patches[0].litter_cs.litr3c = litr3c_mean;
                     patches[0].litter_ns.litr3n = litr3n_mean;

                    }


                // incrament gainer area
                area_sum_g_act3 += patches[0].area; //to do
            }// end skip==2 and no veg
            else
            {
                dG_c3[i] = 0;
                dG_n3[i] = 0;
            }
                //improve what is area_sum_g_act4 < area_sum_g and dG_c_act4 >ZERO; then adjust too
         if (area_sum_g_act3< area_sum_g && dG_c_act3 >ZERO){
             if (command_line[0].verbose_flag == -6)
             printf("\n [Adjust] actual gaining no-veg patches are not all no-veg patches for litter 3");
             litter_c_adjust_total3 += dL_c_act3 /area_sum_g *(area_sum_g - area_sum_g_act3);
             litter_n_adjust_total3 += dL_n_act3 /area_sum_g *(area_sum_g - area_sum_g_act3);
            }

        /* litter 4 */
            if (skip4[i] == 2 &&
                (patches[0].canopy_strata[0][0].defaults[0][0].epc.veg_type) == NON_VEG &&
                 patches[0].litter_cs.litr4c < litr4c_mean &&
                 patches[0].litter_ns.litr4n < litr4n_mean)
            {
                if (command_line[0].verbose_flag == -6)
                    printf("\n || Litter4 gaining ID %d", patches[0].ID);
                // the gain is equally distributed among no-veg patches
                // what if one patch has litter > mean, so the number of gain patches is different from # of no-veg patches

                dG_c4[i] = dL_c_act4 /area_sum_g * patch_area * zone[0].defaults[0][0].sh_litter;// this the mean for each patch todo
                dG_c_act4 += dG_c4[i];
                dG_c_pot4 +=  dL_c_act4 /area_sum_g * patch_area; // here is only one sum_g since it is the sum of all no-veg patches

                dG_n4[i] = dL_n_act4 /area_sum_g * patch_area  * zone[0].defaults[0][0].sh_litter;
                dG_n_act4 += dG_n4[i];
                dG_n_pot4 += dL_n_act4 /area_sum_g * patch_area ;


                if (command_line[0].verbose_flag == -6){
                    if (command_line[0].verbose_flag == -6)
                    printf("\n||[actual litter4 Gain]%f \n", dG_c4[i]);
                    printf("|| [cumulative actual litter4 Gain]%f \n || [cumulative potential litter4 Gain]%f \n", dG_c_act4, dG_c_pot4);
                    printf("|| [before add litter] the litr4c is %f \n", patches[0].litter_cs.litr4c);}

                // litter carbon gain
                patches[0].litter_cs.litr4c += (dG_c4[i] / patch_area);
                patches[0].litter_ns.litr4n += (dG_n4[i] / patch_area);


                if (command_line[0].verbose_flag == -6){
                    printf("\n||[litter 4c gain]%f ", (dG_c4[i] / patch_area));
                    printf("\n||[after add litter], [the litr4c is] %f, [the amount of litter is transfered] %f\n", patches[0].litter_cs.litr4c, (dG_c4[i] / patch_area));}



                  //if the act gain is smaller than potential gain due to parameters, adjust it too ??
                if (dG_c_pot4 > dG_c_act4 && dG_n_pot4 > dG_n_act4)  {
                  if (command_line[0].verbose_flag == -6)
                  printf("\n loop 4 adjust differences between actual gain and potential gain for litter4 [act]%f, [pot]%f \n", dG_c_act4, dG_c_pot4);//improve
                  litter_c_adjust_total4 += (dG_c_pot4 - dG_c_act4);
                  litter_n_adjust_total4 += (dG_n_pot4 - dG_n_act4);

                  }

              // here needs to check if the gaining patches have litter more than mean, then adjust it

                 if (patches[0].litter_cs.litr4c > litr4c_mean &&
                     patches[0].litter_ns.litr4n > litr4n_mean ){


                     litter_c_adjust_total4 += (patches[0].litter_cs.litr4c - litr4c_mean)*patch_area; //to do
                     litter_n_adjust_total4 += (patches[0].litter_ns.litr4n - litr4n_mean)*patch_area;
                     // make the no-veg gain patches litter mean
                     patches[0].litter_cs.litr4c = litr4c_mean;
                     patches[0].litter_ns.litr4n = litr4n_mean;

                    }

                // incrament gainer area
                area_sum_g_act4 += patches[0].area; //to do
            }// end skip==2 and no veg
            else
            {
                dG_c4[i] = 0;
                dG_n4[i] = 0;
            }
        //improve what is area_sum_g_act4 < area_sum_g and dG_c_act4 >0; then adjust too
         if (area_sum_g_act4 < area_sum_g && dG_c_act4 >ZERO){
             if (command_line[0].verbose_flag == -6)
             printf("\n [Adjust] actual gaining no-veg patches are not all no-veg patches for litter 4");
             litter_c_adjust_total4 += dL_c_act4 /area_sum_g *(area_sum_g - area_sum_g_act4);
             litter_n_adjust_total4 += dL_n_act4 /area_sum_g *(area_sum_g - area_sum_g_act4);
            }


        }     // end loop 3


        /*--------------------------------------------------------------*/
        /* Loop 4 , adjust the losing patches (veg patches)             */
        /*--------------------------------------------------------------*/
        if (command_line[0].verbose_flag == -6){
            printf("==============================\n");
            printf("\n|| Adjust litter gain patches loop 4|| \n");
        }


        for (i = 0; i < zone[0].patch_families[pf][0].num_patches_in_fam; i++)
        {

            patches = zone[0].patch_families[pf][0].patches[i];
            patch_area = patches[0].area;
            // adjust the litter to no-veg patches(gainers)
            // litter 1
            if (skip1[i] == 1 &&
                (patches[0].canopy_strata[0][0].defaults[0][0].epc.veg_type) != NON_VEG &&
                 patches[0].litter_cs.litr1c >= litr1c_mean && litter_c_adjust_total1 > ZERO && litter_n_adjust_total1 > ZERO &&
                 patches[0].litter_ns.litr1n >= litr1n_mean)
            {
                if (command_line[0].verbose_flag == -6)
                    printf("\n loop 4 for litter 1 ID %d", patches[0].ID);
                // the adjust is distributed equally among veg patches

                litter_c_adjust1 = litter_c_adjust_total1/area_sum_l1* patch_area;
                litter_n_adjust1 = litter_n_adjust_total1/area_sum_l1* patch_area;


                if (command_line[0].verbose_flag == -6){
                    printf("\n||loop4 adjust [total litter1 adjust*area]%f \n", litter_c_adjust_total1);
                    printf("|| [litter 1c adjust for this patch]%f \n", litter_c_adjust1);
                    printf("|| [before adjust litter1] the litr1c is %f \n",patches[0].litter_cs.litr1c);
                    }

                // litter carbon gain
                patches[0].litter_cs.litr1c += (litter_c_adjust1 / patch_area);
                patches[0].litter_ns.litr1n += (litter_n_adjust1 / patch_area);

                }
            //}
            /* litter 2 */
             if (skip2[i] == 1 &&
                (patches[0].canopy_strata[0][0].defaults[0][0].epc.veg_type) != NON_VEG &&
                 patches[0].litter_cs.litr2c >= litr2c_mean && litter_c_adjust_total2 > ZERO && litter_n_adjust_total2 >ZERO &&
                 patches[0].litter_ns.litr2n >= litr2n_mean)
            {
                if (command_line[0].verbose_flag == -6)
                    printf("\n loop 4 for litter 2 ID %d", patches[0].ID);
                // the adjust is distributed equally among veg patches

                litter_c_adjust2 = litter_c_adjust_total2/area_sum_l2* patch_area;
                litter_n_adjust2 = litter_n_adjust_total2/area_sum_l2* patch_area;


                if (command_line[0].verbose_flag == -6){
                    printf("\n||loop4 adjust [total litter2 adjust*area]%f \n", litter_c_adjust_total2);
                    printf("|| [litter 2c adjust for this patch]%f \n", litter_c_adjust2);
                    printf("|| [before adjust litter2] the litr2c is %f \n",patches[0].litter_cs.litr2c);
                    }

                // litter carbon gain
                patches[0].litter_cs.litr2c += (litter_c_adjust2 / patch_area);
                patches[0].litter_ns.litr2n += (litter_n_adjust2 / patch_area);

                }

            /* litter 3 */
             if (skip3[i] == 1 &&
                (patches[0].canopy_strata[0][0].defaults[0][0].epc.veg_type) != NON_VEG && !isnan(litter_c_adjust_total3) && !isnan(litter_n_adjust_total3) &&
                 patches[0].litter_cs.litr3c >= litr3c_mean && litter_c_adjust_total3 > ZERO && litter_n_adjust_total3 >ZERO && // this is to remove the -nan
                 patches[0].litter_ns.litr3n >= litr3n_mean)
            {
                if (command_line[0].verbose_flag == -6)
                    printf("\n loop 4 for litter 3 ID %d", patches[0].ID);
                // the adjust is distributed equally among veg patches

                litter_c_adjust3 = litter_c_adjust_total3/area_sum_l3* patch_area;
                litter_n_adjust3 = litter_n_adjust_total3/area_sum_l3* patch_area;


                if (command_line[0].verbose_flag == -6){
                    printf("\n||loop4 adjust [total litter3 adjust*area]%f \n", litter_c_adjust_total3);
                    printf("|| [litter 3c adjust for this patch]%f \n", litter_c_adjust3);
                    printf("|| [before adjust litter3] the litr3c is %f \n",patches[0].litter_cs.litr3c);
                    }

                // litter carbon gain
                patches[0].litter_cs.litr3c += (litter_c_adjust3 / patch_area);
                patches[0].litter_ns.litr3n += (litter_n_adjust3 / patch_area);

                }
            /* litter 4 */
             if (skip4[i] == 1 &&
                (patches[0].canopy_strata[0][0].defaults[0][0].epc.veg_type) != NON_VEG &&
                 patches[0].litter_cs.litr4c >= litr4c_mean && litter_c_adjust_total4 > ZERO && litter_n_adjust_total4 > ZERO &&
                 patches[0].litter_ns.litr4n >= litr4n_mean)
            {
                if (command_line[0].verbose_flag == -6)
                    printf("\n loop 4 for litter 4 ID %d", patches[0].ID);
                // the adjust is distributed equally among veg patches

                litter_c_adjust4 = litter_c_adjust_total4/area_sum_l4 * patch_area;
                litter_n_adjust4 = litter_n_adjust_total4/area_sum_l4 * patch_area;


                if (command_line[0].verbose_flag == -6){
                    printf("\n||loop4 adjust [total litter4 adjust*area]%f \n", litter_c_adjust_total4);
                    printf("|| [litter 4c adjust for this patch]%f \n", litter_c_adjust4);
                    printf("|| [before adjust litter4] the litr4c is %f \n",patches[0].litter_cs.litr4c);
                    }

                // litter carbon gain
                patches[0].litter_cs.litr4c += (litter_c_adjust4 / patch_area);
                patches[0].litter_ns.litr4n += (litter_n_adjust4 / patch_area);

                }
            }
        /*--------------------------------------------------------------*/
        /*	Testing -_-  loop 5                                            	*/
        /*--------------------------------------------------------------*/
        /* Initializations */
        area_sum = 0;  //Important, ths initialization should be outside of the loop!

        for (i = 0; i < zone[0].patch_families[pf][0].num_patches_in_fam; i++)
        {


            // if both sh coefficients are not 0, include patch
            if (zone[0].patch_families[pf][0].num_patches_in_fam > 1)
            {

                patches = zone[0].patch_families[pf][0].patches[i];
                if (command_line[0].verbose_flag == -6)
                {
                    printf("\n || Loop 5 After redistribute litter: ID %d | Area %f \n litr1c %f | litr1n %f \n litr2c %f | litr2n %f \n litr3c %f | litr3n %f \n litr4c %f, litr4n %f",
                           patches[0].ID,
                           patches[0].area,
                           patches[0].litter_cs.litr1c,
                           patches[0].litter_ns.litr1n,
                           patches[0].litter_cs.litr2c,
                           patches[0].litter_ns.litr2n,
                           patches[0].litter_cs.litr3c,
                           patches[0].litter_ns.litr3n,
                           patches[0].litter_cs.litr4c,
                           patches[0].litter_ns.litr4n
                           );
                }
                // incrament mean wetness based on storage (rz+unsat or sat) * area
                litr1c_mean_after+= patches[0].litter_cs.litr1c * patches[0].area;
                litr1n_mean_after+= patches[0].litter_ns.litr1n * patches[0].area;
                litr2c_mean_after+= patches[0].litter_cs.litr2c * patches[0].area;
                litr2n_mean_after+= patches[0].litter_ns.litr2n * patches[0].area;
                litr3c_mean_after+= patches[0].litter_cs.litr3c * patches[0].area;
                litr3n_mean_after+= patches[0].litter_ns.litr3n * patches[0].area;
                litr4c_mean_after+= patches[0].litter_cs.litr4c * patches[0].area;
                litr4n_mean_after+= patches[0].litter_ns.litr4n * patches[0].area;
                rooting_depth_mean+= patches[0].rootzone.depth * patches[0].area;


                // area sum (patch fam without skipped patches)
                area_sum += patches[0].area;



            }

        } // end loop 5

        // Get mean wetness - vol water/(total patch family) area - units are meters depth
        if (area_sum > ZERO)
        {
            litr1c_mean_after/= area_sum;
            litr1n_mean_after/= area_sum;
            litr2c_mean_after/= area_sum;
            litr2n_mean_after/= area_sum;
            litr3c_mean_after/= area_sum;
            litr3n_mean_after/= area_sum;
            litr4c_mean_after/= area_sum;
            litr4n_mean_after/= area_sum;
            rooting_depth_mean /= area_sum;
        }
        else
        {
            litr1c_mean_after= 0.0;
            litr1n_mean_after= 0.0;
            litr2c_mean_after= 0.0;
            litr2n_mean_after= 0.0;
            litr3c_mean_after= 0.0;
            litr3n_mean_after= 0.0;
            litr4c_mean_after= 0.0;
            litr4n_mean_after= 0.0;
            rooting_depth_mean = 0.0;
        }

        if (command_line[0].verbose_flag == -6){
            printf("\n**||After lateral redistribution Mean litter1c = %f|| Mean litter1n = %f ", litr1c_mean_after, litr1n_mean_after);
            printf("\n**||After lateral redistribution Mean litter2c = %f|| Mean litter2n = %f ", litr2c_mean_after, litr2n_mean_after);
            printf("\n**||After lateral redistribution Mean litter3c = %f|| Mean litter3n = %f ", litr3c_mean_after, litr3n_mean_after);
            printf("\n**||After lateral redistribution Mean litter4c = %f|| Mean litter4n = %f ", litr4c_mean_after, litr4n_mean_after);

            printf("\n ||Mean root depth is %f \n", rooting_depth_mean);
            printf("==============================\n");
            }

        /* check the carbon balance */

        if ((litr1c_mean_after - litr1c_mean) > ZERO || (litr1n_mean_after - litr1n_mean) > ZERO)
            printf("\nWARNING: carbon is not balanced for litter 1 for patch family %d, before share litr1c = %f | after litr1c = %f",
                    zone[0].patch_families[pf][0].family_ID, litr1c_mean, litr1c_mean_after);
        if ((litr2c_mean_after - litr2c_mean) > ZERO || (litr2n_mean_after - litr2n_mean) > ZERO)
            printf("\nWARNING: carbon is not balanced for litter 2 for patch family %d, before share litr2c = %f | after litr2c = %f",
                    zone[0].patch_families[pf][0].family_ID, litr2c_mean, litr2c_mean_after);
        if ((litr3c_mean_after - litr3c_mean) > ZERO || (litr3n_mean_after - litr3n_mean) > ZERO)
            printf("\nWARNING: carbon is not balanced for litter 3 for patch family %d, before share litr3c = %f | after litr3c = %f",
                    zone[0].patch_families[pf][0].family_ID, litr3c_mean, litr3c_mean_after);
        if ((litr4c_mean_after - litr4c_mean) > ZERO || (litr4n_mean_after - litr4n_mean) > ZERO)
            printf("\nWARNING: carbon is not balanced for litter4 for patch family %d, before share litr4c = %f | after litr4c = %f",
                    zone[0].patch_families[pf][0].family_ID, litr4c_mean, litr4c_mean_after);
        if (command_line[0].verbose_flag == -6)
        printf("\n end of routing litter \n ==============================\n");

    } // end patch family loop

    return;
}