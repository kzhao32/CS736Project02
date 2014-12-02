/*
 * Copyright 2007 Yusuke Yamamoto
 * Copyright 2014 Kai Zhao
 * 
 * This java program extracts live, public Twitter Stream data into a file.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.RandomAccessFile;

import twitter4j.StallWarning;
import twitter4j.Status;
import twitter4j.StatusDeletionNotice;
import twitter4j.StatusListener;
import twitter4j.TwitterException;
import twitter4j.TwitterStream;
import twitter4j.TwitterStreamFactory;

/**
 * <p>This is a code example of Twitter4J Streaming API - sample method support.<br>
 * Usage: java twitter4j.examples.PrintSampleStream<br>
 * </p>
 *
 * @author Yusuke Yamamoto - yusuke at mac.com
 */
public final class collector {
	/**
	 * Main entry of this application.
	 */
	static int numberOfTweets = 0;
	static int maxLength = 0;
	public static void main(String[] args) throws TwitterException, FileNotFoundException {
		if (args.length == 0) {
			String[] argsInput = { "bonsai.dat", "1000" };
			args = argsInput;
		}
		if (args.length != 2) {
			System.err.println("Usage: java PrintSampleStream.java <output> <numberOfTweets>");
			System.exit(1);
		}

		// create a random access file. Random access gives faster write speeds
		final RandomAccessFile outputFile = new RandomAccessFile(new File(args[0]), "rwd");

		final int timeMillisecLength = 13 + "\0".length(); 	//"1415927476669".length() == 13;
		final int timeDateLength = 28 + "\0".length(); 		// "Mon Dec 01 14:48:17 CST 2014".length() == 28
		final int screenNameLength = 15 + "\0".length(); 	// max allowable by Twitter
		final int textLength = 140 + "\0".length();			// max allowable by Twitter
		final int numberOfFollowersLength = 9 + "\0".length();	// more than max currently. Katy Perry has 61 million followers as of Dec 2014
		final int totalTweetLength = timeMillisecLength + timeDateLength + screenNameLength + textLength + numberOfFollowersLength + "\n".length();
		final int totalNumberOfTweets = Integer.parseInt(args[1]); // initialize
		long oldestDate = Long.MAX_VALUE;					// initialize oldest date to see where to start replacing
		int indexOfOldestDate = 0;
		for (int i = 0; i < totalNumberOfTweets; ++i) {		// check through all the tweets
			byte[] bytesRead = new byte[timeMillisecLength];// buffer to read timeMillisec
			try {
				outputFile.seek(i * totalTweetLength);		// seek to timeMillisec
				outputFile.read(bytesRead, 0, timeMillisecLength); // string to store, starting store, number of bytes to read
				long indexDate = Long.parseLong(new String(bytesRead, "UTF-8").trim()); // trim to truncate null space
				if (indexDate < oldestDate) {				// if older index date
					indexOfOldestDate = i;					// then update index of oldest date
					oldestDate = indexDate;
				}
			} catch (IOException | NumberFormatException e) { // if incorrect format
				break;										// default to oldestIndexDate = 0
			}
		}
		numberOfTweets = indexOfOldestDate;					// set index of oldest tweet to start replacing
		try {
			System.out.println("indexOfOldestTweet = " + numberOfTweets);
			outputFile.seek(indexOfOldestDate * totalTweetLength);
		} catch (IOException e) { 
			e.printStackTrace();
		}

		TwitterStream twitterStream = new TwitterStreamFactory().getInstance();
		StatusListener listener = new StatusListener() {	// define listener
			@Override
			public void onStatus(Status status) {
				// extract variables
				String timeMillisec = "" + status.getTimeStamp() + '\0';
				String timeDate = "" + status.getCreatedAt() + '\0';
				String screenName = status.getUser().getScreenName() + '\0';
				String text = status.getText() + '\0';
				String numberOfFollowers = "" + status.getUser().getFollowersCount() + '\0';

				try {
					// running length is the seek position
					int runningLength = numberOfTweets * totalTweetLength;

					outputFile.seek(runningLength);			// seek
					outputFile.writeBytes(timeMillisec);	// write variable
					runningLength += timeMillisecLength;	// increment seek index
															// repeat
					outputFile.seek(runningLength);
					outputFile.writeBytes(timeDate);
					runningLength += timeDateLength;
					
					outputFile.seek(runningLength);
					outputFile.writeBytes(screenName);
					runningLength += screenNameLength;
					
					outputFile.seek(runningLength);
					outputFile.writeBytes(text);
					runningLength += textLength;
					
					outputFile.seek(runningLength);
					outputFile.writeBytes(numberOfFollowers);
					runningLength += numberOfFollowersLength;
					
					outputFile.seek(runningLength);
					outputFile.writeBytes("\n");
				} catch (IOException e) {
					e.printStackTrace();
				}

				if (++numberOfTweets >= totalNumberOfTweets) {
					System.out.println("done.");
					try {
						outputFile.close();
					} catch (IOException e) {
						e.printStackTrace();
					}
					System.exit(0);
				}
			}

			@Override
			public void onDeletionNotice(StatusDeletionNotice statusDeletionNotice) {
				//                System.out.println("Got a status deletion notice id:" + statusDeletionNotice.getStatusId());
			}

			@Override
			public void onTrackLimitationNotice(int numberOfLimitedStatuses) {
				//                System.out.println("Got track limitation notice:" + numberOfLimitedStatuses);
			}

			@Override
			public void onScrubGeo(long userId, long upToStatusId) {
				//                System.out.println("Got scrub_geo event userId:" + userId + " upToStatusId:" + upToStatusId);
			}

			@Override
			public void onStallWarning(StallWarning warning) {
				//                System.out.println("Got stall warning:" + warning);
			}

			@Override
			public void onException(Exception ex) {
				//                ex.printStackTrace();
			}
		};
		twitterStream.addListener(listener);
		twitterStream.sample();
	}
}
