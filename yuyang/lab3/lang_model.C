
//  $Id: lang_model.C,v 1.9 2009/10/15 20:59:13 stanchen Exp $


#include "lang_model.H"


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void LangModel::write_counts(const string& fileName) const
    {
    ofstream outStrm(fileName.c_str());
    outStrm << "# Pred counts.\n";
    m_predCounts.write(outStrm, get_sym_table());
    outStrm << "# Hist counts.\n";
    m_histCounts.write(outStrm, get_sym_table());
    outStrm << "# Hist 1+ counts.\n";
    m_histOnePlusCounts.write(outStrm, get_sym_table());
    outStrm.close();
    }

void LangModel::count_sentence_ngrams(const vector<int>& wordList)
    {
    int wordCnt = wordList.size();

    //
    //  BEGIN_LAB
    //
    //  This routine is called for each sentence in the training
    //  data.  It should collect all relevant n-gram counts for
    //  the sentence.
    //
    //  Input:
    //      "m_n" = the value of "n" for the n-gram model; e.g.,
    //          this has the value 3 for a trigram model.
    //      "wordCnt" = the number of words in the vector "wordList".
    //
    //      The vector "wordList[0 .. (wordCnt-1)]" holds the sentence
    //      as a sequence of integer indices (each integer
    //      representing a word).  This vector is padded with
    //      beginning-of-sentence and end-of-sentence markers in
    //      a convenient manner for counting.  In particular,
    //      "wordList[0 .. (m_n - 2)]" holds beginning-of-sentence markers, so
    //      the first "real" word of the sentence is "wordList[m_n - 1]";
    //      and "wordList[wordCnt-1]" holds the end-of-sentence marker.
    //
    //  Output:
    //      Update all the counts needed for Witten-Bell smoothing.
    //
    //      Specifically, the objects "m_predCounts", "m_histCounts", and
    //      "m_histOnePlusCounts" are all of type "NGramCounter", a class
    //      that can be used to store counts for a set of n-grams.
    //      The object "m_predCounts" should be used to store
    //      "regular" n-gram counts; the object "m_histCounts" should
    //      be used to store "history" n-gram counts (i.e., the
    //      normalization count for that n-gram occuring as a history);
    //      and "m_histOnePlusCounts" should be used to store the
    //      number of different words following that n-gram history.
    //      To increment the count of a particular n-gram in
    //      one of these objects, you can do a call like:
    //
    //      m_predCounts.incr_count(ngram);
    //
    //      where "ngram" is of type "vector<int>".  This call returns
    //      the value of the incremented count.
    //
    //      Your code should work for any value of m_n (larger than zero).
   
    vector<int> temp_eps(1);
    temp_eps[0]=0;
    for (int m_n_t=1;m_n_t<=m_n;++m_n_t)
      {	
	for(int i=m_n-1;i<wordCnt;++i)
	  {
	    vector<int> temp(m_n_t);
	    vector<int> temp_hist(m_n_t-1);
	    for (int j=0; j<m_n_t;++j)
	      {
		temp[j]=wordList[i-(m_n_t-1)+j];
		if ((j<m_n_t-1) && (m_n_t>1))
		  {
		    temp_hist[j]=wordList[i-(m_n_t-1)+j];
		  }
	      }
	   m_predCounts.incr_count(temp);
	   if (m_n_t>1) m_histCounts.incr_count(temp_hist);
	   if (m_n_t==1) m_histCounts.incr_count(temp_eps);
	   if ((m_n_t>1)&&(m_predCounts.get_count(temp)==1)) m_histOnePlusCounts.incr_count(temp_hist);
	   if ((m_n_t==1)&&(m_predCounts.get_count(temp)==1)) m_histOnePlusCounts.incr_count(temp_eps);
	  }

      }
    //  END_LAB
    //
    }

LangModel::LangModel(const map<string, string>& params) : m_params(params),
    m_symTable(new SymbolTable(get_required_string_param(m_params, "vocab"))),
    m_bosIdx(m_symTable->get_index(get_string_param(m_params, "bos", "<s>"))),
    m_eosIdx(m_symTable->get_index(get_string_param(m_params, "eos", "</s>"))),
    m_unkIdx(m_symTable->get_index(
        get_string_param(m_params, "unk", "<UNK>"))),
    m_n(get_int_param(m_params, "n", 3)),
    m_delta(get_float_param(m_params, "delta", -1.0))
    {
    if ((m_bosIdx == -1) || (m_eosIdx == -1) || (m_unkIdx == -1))
        throw runtime_error("Vocabulary missing BOS/EOS/UNK token.");

    //  Get training counts.
    ifstream inStrm(
        get_required_string_param(params, "train").c_str());
    string lineStr;
    vector<string> wordList;
    vector<int> wordIdxList;
    while (inStrm.peek() != EOF)
        {
        //  Read line, split into tokens, convert to indices, count n-grams.
        getline(inStrm, lineStr);
        split_string(lineStr, wordList);
        convert_words_to_indices(wordList, wordIdxList, get_sym_table(),
            get_ngram_length(), get_bos_index(), get_eos_index(),
            get_unknown_index());
        count_sentence_ngrams(wordIdxList);
        }

    string countFile = get_string_param(params, "count_file");
    if (!countFile.empty())
        write_counts(countFile);
    }

double LangModel::get_prob_plus_delta(const vector<int>& ngram) const
    {
    double retProb = 1.0;
    //  Don't count epsilon.
    int vocSize = m_symTable->size() - 1;

    //
    //  BEGIN_LAB
    //
    //  This routine should return an n-gram probability smoothed
    //  with plus-delta smoothing.
    //
    //  Input:
    //      "m_n" = the value of "n" for the n-gram model.
    //      "ngram" = the input n-gram, expressed as a vector of integers.
    //          This will be of length "m_n".
    //      "m_predCounts" = object holding "regular" counts for each n-gram.
    //      "m_histCounts" = object holding "history" counts for each n-gram.
    //      "vocSize" = vocabulary size.
    //      "m_delta" = value of delta.
    //
    //      To access the count of an n-gram, you can do something like:
    //
    //      int predCnt = m_predCounts.get_count(ngram);
    //
    //      where "ngram" is of type "vector<int>".
    //
    //  Output:
    //      "retProb" should be set to the smoothed n-gram probability
    //          of the last word in the n-gram given the previous words.
    //
    if (m_n>1)
      {
	vector<int>temp(m_n-1);
	for(int i=0;i<m_n-1;++i) temp[i]=ngram[i];
	retProb=(m_predCounts.get_count(ngram)+m_delta)/(m_histCounts.get_count(temp)+m_delta*vocSize*1.0);
      }
    else
      {
	vector<int>temp_eps(1);
	temp_eps[0]=0;
	retProb=(m_predCounts.get_count(ngram)+m_delta)/(m_histCounts.get_count(temp_eps)+m_delta*vocSize*1.0);
      }
    //  END_LAB
    //

    return retProb;
    }

double LangModel::get_prob_witten_bell(const vector<int>& ngram) const
    {
    double retProb = 1.0;
    //  Don't count epsilon.
    int vocSize = m_symTable->size() - 1;

    //
    //  BEGIN_LAB
    //
    //  This routine should return an n-gram probability smoothed
    //  with Witten-Bell smoothing.
    //
    //  Input:
    //      "m_n" = the value of "n" for the n-gram model.
    //      "ngram" = the input n-gram, expressed as a vector of integers.
    //          This will be of length "m_n".
    //      "m_predCounts" = object holding "regular" counts for each n-gram.
    //      "m_histCounts" = object holding "history" counts for each n-gram.
    //      "m_histOnePlusCounts" = object holding number of unique words
    //          following each n-gram history in the training data.
    //      "vocSize" = vocabulary size.
    //
    //      To access the count of an n-gram, you can do something like:
    //
    //      int predCnt = m_predCounts.get_count(ngram);
    //
    //      where "ngram" is of type "vector<int>".
    //
    //  Output:
    //      "retProb" should be set to the smoothed n-gram probability
    //          of the last word in the n-gram given the previous words.
    //
    vector<double> p_MLE(m_n-1);
    vector<double> p_WB(m_n);
    vector<int> temp_eps(1);
    temp_eps[0]=0;
    int eps_hist(m_histCounts.get_count(temp_eps));
    int eps_plus(m_histOnePlusCounts.get_count(temp_eps));
    
    for (int i=1;i<=m_n;++i)
      {
	if (i==1)
	  {
	    //calculate P_wb(w_i)
	    vector<int> temp(i);
	    temp[0]=ngram[m_n-i];
	    p_WB[i-1]=(1.0*eps_hist/(eps_hist+eps_plus))*(1.0*m_predCounts.get_count(temp)/m_histCounts.get_count(temp_eps))+(1.0*eps_plus/(eps_plus+eps_hist))/vocSize;
	    // if (m_n==2) cout<<format("vocSize %i word:%i pred %i ,hist %i, plus %i, wb %d %d \n")%vocSize %temp[0] %m_histCounts.get_count(temp) %eps_hist %eps_plus %p_WB[i-1] %(1.0*eps_plus/(eps_plus+eps_hist));
	  }
	else
	  {
	    //calculate P_MLE and P_WB
	    vector<int> temp(i);
	    vector<int> last_temp(i-1);
	    for (int j=0;j<i;++j)
	      {
		temp[j]=ngram[m_n-i+j];
	      }
	    for (int j=0;j<i-1;++j)
	      {
		last_temp[j]=temp[j];
	      }
	    int temp_hist(m_histCounts.get_count(last_temp));
	    int temp_plus(m_histOnePlusCounts.get_count(last_temp));
	    if(temp_hist==0)
	      {
		p_MLE[i-2]=0;
		p_WB[i-1]=p_WB[i-2];
	      }
	    else
	      {
		 p_MLE[i-2]=1.0*m_predCounts.get_count(temp)/m_histCounts.get_count(last_temp);
		 p_WB[i-1]=(1.0*temp_hist/(temp_hist+temp_plus))*p_MLE[i-2]+(1.0*temp_plus/(temp_hist+temp_plus))*p_WB[i-2];
	      }
	   
	    
	  }
      }
    //
    /*if(m_n==2)
      {
	vector<int> temp(2);
	vector<int> last_temp(1);
	temp[0]=ngram[0];
	temp[1]=ngram[1];
	last_temp[0]=ngram[0];
	cout<<format("C(w_i)=%i, C_h(w_i)=%i, C_p(w_i)=%i\n")%m_predCounts.get_count(last_temp) %m_histCounts.get_count(last_temp) %m_histOnePlusCounts.get_count(last_temp);
	cout<<format("C(w_i|w_i-1)=%i, C_h(w_i|w_i-1)=%i, C_p(w_i|w_i-1)=%i\n")%m_predCounts.get_count(temp) %m_histCounts.get_count(temp) %m_histOnePlusCounts.get_count(temp);
      }
    if (m_n==2) cout<<format("\nword is:");
    for (int i=0;i<m_n;++i)
      {
	if (m_n==2) cout<<format("%i ")%ngram[i];
      }
    if (m_n==2) cout<<format("\n");
    if (m_n==2) cout<<format("MLE: ");
    for (int i=0;i<p_MLE.size();++i)
      {
	if (m_n==2) cout<<format("%d ")%p_MLE[i];
      }
    if (m_n==2) cout<<format("\nWB:");
    for (int i=0;i<p_WB.size();++i)
      {
	if (m_n==2) cout<<format("%d ")%p_WB[i];
      }
    if (m_n==2) cout<<format("\n");
    */
    retProb=p_WB[m_n-1];



    //  END_LAB
    //

    return retProb;
    }


double LangModel::get_prob(const vector<int>& ngram) const
    {
    if ((ngram.size() < 1) || ((int) ngram.size() > m_n))
        throw runtime_error("Invalid n-gram size.");
    return (m_delta >= 0.0) ? get_prob_plus_delta(ngram) :
        get_prob_witten_bell(ngram);
    }


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


