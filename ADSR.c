#include<stdio.h>
#include<stdlib.h>

/* Each sample data consists of time and amplitude data*/
struct sample
{
	float time;
	float amp;
};

/* Different phases of the ADSR envelope */
enum state
{
	initial = 0,
	attack = 1,
	decay = 2,
	sustain = 3,
	release = 4,
	end = 5,
};

/* Structure for storing parameters required for ADSR envelope */
struct ADSR_env
{
	/* Time length of attack phase */
	float a_t;

	/* Time length of decay time */
	float d_t;

	/* Time length of sustain time */
	float s_t;

	/* Time length of release time */
	float r_t;

	/* Peak amplitude during attack phase */
	float attack_amp;

	/* Amplitude to which it decays during the decay phase */
	float decay_amp;

	/* Total samples in ADSR envelope */
	float N_samples;

	/* Total time length of the ADSR envelope */
	float t_total;

	/* sampling period */
	float ts;

	/* pointer to ADSR envelope output samples */
	struct sample *adsr_samples;
};

/* This function creates an ADSR envelope based on the input parameters
   like the time length for the attack, decay, sustain and release phases.
   The linear increase or decrease of the envelope is calculated based on
   the slope of the line.
   Slope of the line is given by :
   slope = (target_amplitude - initial amplitude)/(target_time - initial_time)
   */
create_ADSR(struct ADSR_env *adsr)
{
	float slope;
	int i;
	enum state curr_state = 0;
	struct sample *out_samples = adsr->adsr_samples;

	/* Required slope of the line is calculated using the formula :
	slope = (target_amplitude - initial amplitude) / (target_time - initial_time)
	*/
	slope = (adsr->attack_amp - 0.0) / (adsr->a_t - 0.0);

	/* Loops for each sample in the ADSR envelope */
	for (i = 0; i < adsr->N_samples; i++)
	{
		switch (curr_state)
		{
			/* initial phase : it only executes this part once
			   Amplitude and time are set to zero for the first
			   sample.
			   It next enters the attack phase. So the state is set to
			   attack
			   */
		case initial:  curr_state = attack;
			out_samples->amp = 0.0;
			out_samples->time = 0.0;
			out_samples++;
			break;

			/* Attack phase : It is the phase when the amplitude increases linearly
			   and reaches the maximum amplitude set. The time and amplitude value
			   for each sample is updated. When the amplitude reaches the
			   maximum amplitude, curr_state is changed to decay.*/
		case attack:  out_samples->amp = ((out_samples - 1)->amp) + slope * adsr->ts;
			out_samples->time = (out_samples - 1)->time + adsr->ts;
			if (out_samples->amp >= adsr->attack_amp)
				curr_state = decay;
			out_samples++;
			break;

			/* Decay phase : It is the phase where amplitude reduces a little to the set value.
			   The amplitude and time values are updated for each sample. When it decreases to
			   the desired amplitude, the curr_state is set to sustain and then it goes to the
			   sustain state */
		case decay:  out_samples->amp = ((out_samples - 1)->amp) - slope * adsr->ts;
			out_samples->time = (out_samples - 1)->time + adsr->ts;
			if ((out_samples->amp) <= adsr->decay_amp)
				curr_state = sustain;
			out_samples++;
			break;

			/* Sustain phase : It is the phase in which the amplitude is maintained at a constant
			   value for the user decided amount of time. After the desired length of time, the
			   state is set  to  release */
		case sustain:  out_samples->amp = ((out_samples - 1)->amp);
			out_samples->time = (out_samples - 1)->time + adsr->ts;
			if ((out_samples->time) >= (adsr->a_t + adsr->d_t + adsr->s_t))
				curr_state = release;
			out_samples++;
			break;

			/* Release phase : In this phase the amplitude decreases linearly to zero. When the
			   amplitude reaches zero, the release phase terminates and the curr_state is set to
			   attack as it starts annother ADSR loop */
		case release:  out_samples->amp = ((out_samples - 1)->amp) - slope * adsr->ts;
			out_samples->time = (out_samples - 1)->time + adsr->ts;
			if (out_samples->amp <= 0)
				curr_state = attack;
			out_samples++;
			break;

		}
	}
}

void main()
{
	struct ADSR_env adsr;
	float fs;
	float adsr_length, num_samples;
	FILE *fp_adsr, *fp_in, *fp_out;
	int i;

	float time, amp;
	float time_syn, amp_syn;

	/* opening the output file for ADSR envelope */
	fp_adsr = fopen("C:\\Anu\\academics\\Computer_Audio\\research_paper\\implementation\\adsr_out.dat",
		"w+");

	/* opening the output file for ADSR envelope */
	fp_in = fopen("C:\\Anu\\academics\\Computer_Audio\\research_paper\\implementation\\input_audio.dat",
		"r+");

	/* opening the output file for ADSR envelope */
	fp_out = fopen("C:\\Anu\\academics\\Computer_Audio\\research_paper\\implementation\\modulated_out.dat",
		"w+");

	/* Takes the time taken for attack, decay, sustain and release
	   from the user */
	printf("Enter the time for attack, decay, sustain and release");
	scanf("%f%f%f%f", &adsr.a_t, &adsr.d_t, &adsr.s_t, &adsr.r_t);

	/* Sampling rate required for the ADSR envelope signal is taken
	   from the user*/
	printf("Enter the sampling rate of the input signal\n");
	scanf("%f", &fs);


	//adsr.a_t = 0.1;
	//adsr.d_t = 0.05;
	//adsr.s_t = 0.5;
	//adsr.r_t = 0.05;

	/* calculating the total length of the ADSR envelope */
	adsr.t_total = adsr.a_t + adsr.d_t + adsr.s_t + adsr.r_t;

	/* Calculating the total number of samples in ADSR envelope */
	adsr.N_samples = adsr.t_total * fs;

	/* Calculation of the sampling time period */
	adsr.ts = 1 / fs;

	/* Setting the maximum amplitude of the attack phase and the
	   amplitude to which it decays */
	adsr.attack_amp = 1.0;
	adsr.decay_amp = 0.6;

	/* Allocating the memory for ADSR samples */
	adsr.adsr_samples = (struct sample *)malloc(sizeof(struct sample) * adsr.N_samples);

	/* creates an ADSR envelope based on the user's parameters */
	create_ADSR(&adsr);

	/* writing the ADSR envelope samples to output .dat file */
	for (i = 0; i < adsr.N_samples; i++)
	{
		fprintf(fp_adsr, "%f\t%f\n", (adsr.adsr_samples + i)->time, (adsr.adsr_samples + i)->amp);
	}

	/* Amplitude modulation */
	for (i = 0; i < adsr.N_samples; i++)
	{
		/* reading the input sample values from file */
		fscanf(fp_in, "%f%f\n", &time, &amp);

		time_syn = (adsr.adsr_samples + i)->time;

		/* multiplying the adsr envelope with the envelope */
		amp_syn = (adsr.adsr_samples + i)->amp * amp;

		/* Writing the amplitude modulated signal to output file */
		fprintf(fp_out, "%f\t%f\n", time_syn, amp_syn);
	}


}