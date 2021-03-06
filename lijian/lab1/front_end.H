
//  $Id: front_end.H,v 1.5 2009/09/14 21:17:41 stanchen Exp $

/** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **
*   @file front_end.H
*   @brief Contains FrontEnd class for signal processing.
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef _FRONT_END_H
#define _FRONT_END_H


#include "util.H"


/** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **
*   Front end class; does signal processing.
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
class FrontEnd
    {
    public:
        /** Ctor; get configuration parameters from @p params. **/
        FrontEnd(const map<string, string>& params = ParamsType()) :
            m_params(params) { }

        /** Process input signal from @p inAudio; place acoustic
        *   feature vectors in @p outFeats.
        **/
        void get_feats(const matrix<double>& inAudio,
            matrix<double>& outFeats) const;

    private:
        /** Do windowing. **/
        void do_window(const matrix<double>& inFeats,
            matrix<double>& outFeats) const;

        /** Do FFT. **/
        void do_fft(const matrix<double>& inFeats,
            matrix<double>& outFeats) const;

        /** Do Mel binning. **/
        void do_melbin(const matrix<double>& inFeats,
            matrix<double>& outFeats) const;

        /** Do discrete cosine transform. **/
        void do_dct(const matrix<double>& inFeats,
            matrix<double>& outFeats) const;

    private:
        /** Stores copy of input parameters. **/
        map<string, string> m_params;
    };


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#endif


